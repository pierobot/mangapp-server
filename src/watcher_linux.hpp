#ifndef WATCHER_WIN_HPP
#define WATCHER_WIN_HPP

#include "watcher.hpp"
#include "utf8.hpp"

#include <chrono>
#include <iostream>
#include <iterator>
#include <map>

#include <linux/version.h>
#include <errno.h>
// Make sure inotify is supported by the kernel
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,16)
//  Display an error if not
#   error "A minimum linux kernel of version 2.6.16 is required for inotify."
//  Eventually, there will be an implementation without inotify.
#endif
#include <sys/inotify.h>
#include <unistd.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/functional/hash.hpp>

extern void console_write_line(std::string const & function_name, std::string const & message);

namespace mangapp
{
    class watcher_linux : public watcher
    {
    public:
        watcher_linux(on_file_event on_path_change, on_file_event on_file_change, on_directory_fail_event on_directory_fail) :
            watcher(on_path_change, on_file_change, on_directory_fail),
            m_fd(::inotify_init1(IN_NONBLOCK)),
            m_main_paths(),
            m_paths()

        {
            if (m_fd == -1)
                console_write_line("watcher_linux::watcher_linux", std::string("Call to inotify_init1 failed. ") + strerror(errno));
        }

        virtual ~watcher_linux()
        {
        }

        virtual void start() final
        {
            if (m_fd == -1)
            {
                console_write_line("watcher_linux::start", "m_fd is not a valid descriptor.");
                return;
            }

            m_thread = std::thread([this]()
            {
                // Allocate a buffer large enough for at least 10 notifications
                uint8_t buffer[16 * sizeof(inotify_event) + NAME_MAX + 1];

                //console_write_line("watcher_linux::start", "Watching for notifications.");

                while (m_continue_running == true)
                {
                    // Check for any available notifications
                    int bytes_read = ::read(m_fd, buffer, sizeof(buffer));
                    if (bytes_read == -1)
                    {
                        if (errno == EAGAIN)
                        {
                            // No data available yet, try again
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    	                    continue;
                        }

                        console_write_line("watcher_linux::start::thread", std::string("Call to read failed.") + strerror(errno));
                        break;
                    }

                    size_t offset = 0;
                    inotify_event const * notification = nullptr;
                    do
                    {
                        notification = reinterpret_cast<inotify_event const *>(&buffer[offset]);

                        // Make sure the path descriptor exists in our map
                        auto path_iterator = m_paths.cend();
                        {
                            std::lock_guard<std::mutex> lock(m_mutex);
                            path_iterator = m_paths.find(notification->wd);
                        }

                        if (path_iterator != m_paths.cend())
                        {
                            std::string utf8_name;
                            std::copy(&notification->name[0], &notification->name[notification->len], std::back_inserter(utf8_name));
                            // notification->len includes trailing NULL characters. Trim those when converting to utf16
                            std::wstring name = to_utf16(utf8_name.begin(), std::find(utf8_name.cbegin(), utf8_name.cend(), '\0'));
                            bool needs_trailing_slash = path_iterator->second.back() != '/' ? true : false;
                            std::wstring const & path = needs_trailing_slash == true ? path_iterator->second + L"/" : path_iterator->second;
                            size_t key = -1;

                            // Get the name of the directory_entry in our library
                            for (auto const & main_path : m_main_paths)
                            {
                                size_t name_start_pos = path.find(main_path);
                                if (name_start_pos != std::wstring::npos)
                                {
                                    name_start_pos += main_path.length();
                                    size_t name_end_pos = path.find('/', name_start_pos);
                                    if (name_end_pos != std::wstring::npos)
                                    {
                                        auto library_name = path.substr(name_start_pos, name_end_pos - name_start_pos);
                                        key = boost::hash<std::wstring>()(library_name);

                                        break;
                                    }
                                }
                            }

                            boost::system::error_code ec;
                            bool is_directory = boost::filesystem::is_directory(path + name, ec);
                            if (is_directory == false && ec != 0)
                            {
                                is_directory = m_on_directory_fail(name);
                            }

                            switch (notification->mask & 0x0000ffff)
                            {
                            case IN_CREATE:
                            case IN_MOVED_TO:
                            {
                                if (is_directory == true)
                                    add_subdirectories(path);

                                if (m_on_path_change != nullptr)
                                    m_on_path_change(path, name, is_directory, false, key);

                                break;
                            }

                            case IN_DELETE:
                            case IN_MOVED_FROM:
                            {
                                if (is_directory == true)
                                    remove_path(path);

                                if (m_on_path_change != nullptr)
                                    m_on_path_change(path, name, is_directory, true, key);

                                break;
                            }

                            default:
                                break;
                            }
                        }
                        else
                            console_write_line("watcher_linux::start::thread", "Unable to find path associated with wd.");

                        offset += sizeof(inotify_event) + notification->len;
                    } while (offset < bytes_read);
                }

                // If execution reaches this point, the thread is shutting down.
                m_paths.clear();
                m_main_paths.clear();
            });
        }

        virtual void stop() final
        {
            for (auto const & kv : m_paths)
            {
                close(kv.first);
            }

            m_continue_running = false;
            close(m_fd);
        }

        virtual void add_path(std::wstring const & path) final
        {
            uint32_t mask = IN_CREATE | IN_DELETE | IN_MOVE | IN_UNMOUNT | IN_DELETE_SELF | IN_DONT_FOLLOW;

            std::string utf8_path = to_utf8(path);
            int wd = ::inotify_add_watch(m_fd, utf8_path.c_str(), mask);
            if (wd != -1)
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_paths.emplace(wd, path);
                    m_main_paths.push_back(path);
                }

                add_subdirectories(path);
            }
            else
            {
                console_write_line("watcher_linux::add_path", std::string("Call to inotify_add_watch failed.") + strerror(errno));
            }
        }

        void add_subdirectories(std::wstring const & path)
        {
            uint32_t mask = IN_CREATE | IN_DELETE | IN_MOVE | IN_UNMOUNT | IN_DELETE_SELF | IN_DONT_FOLLOW;

            std::string utf8_path = to_utf8(path);
            int wd = ::inotify_add_watch(m_fd, utf8_path.c_str(), mask);
            if (wd != -1)
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_paths.emplace(wd, path);
                }

                boost::system::error_code ec;
                using directory_iterator = boost::filesystem::recursive_directory_iterator;
                // Iterate through the subdirectories
                for (auto directory = directory_iterator(path, ec); directory != directory_iterator(); ++directory)
                {
                    // Do we have access to the file/directory?
                    auto status = directory->status(ec);
                    if (ec != 0)
                    {
                        console_write_line("watcher_linux::add_subdirectories", std::string("boost::filesystem::status failed. ") + ec.message());
                        continue;
                    }

                    bool is_directory = boost::filesystem::is_directory(directory->path(), ec);
                    if (ec != 0)
                    {
                        console_write_line("watcher_linux::add_subdirectories", std::string("boost::filesystem::is_directory failed. ") + ec.message());
                    }

                    // We're only interested in watching directories; is_directory also returns false if ec != 0
                    if (is_directory == false)
                        continue;

                    add_subdirectories(directory->path().generic_wstring());

                    //console_write_line("watcher_linux::add_subdirectories", std::string("Watching for events in ") + directory->path().generic_string());
                }

                if (ec != 0)
                {
                    console_write_line("watcher_linux::add_subdirectories", std::string("Unable to iterate directory. ") + ec.message());
                }
            }
            else
            {
                console_write_line("watcher_linux::add_path", std::string("Call to inotify_add_watch failed.") + strerror(errno));
            }
        }

        virtual void remove_path(std::wstring const & path) final
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto const path_iterator = std::find_if(m_paths.cbegin(), m_paths.cend(),
                [&path](std::pair<int, std::wstring> const & kv)
            {
                return kv.second == path;
            });

            if (path_iterator != m_paths.cend())
                ::inotify_rm_watch(m_fd, path_iterator->first);
        }

    protected:
    private:
        int m_fd;
        std::vector<std::wstring> m_main_paths;
        std::map<int, std::wstring> m_paths;
    };
}

#endif