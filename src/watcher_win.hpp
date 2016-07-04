#ifndef WATCHER_WIN_HPP
#define WATCHER_WIN_HPP

#include "watcher.hpp"

#include <Windows.h>

#include <atomic>
#include <iterator>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/filesystem.hpp>

extern void console_write_line(std::string const & function_name, std::string const & message);

static bool enable_privilege(std::string const & name)
{
    HANDLE token_handle = INVALID_HANDLE_VALUE;
    bool result = false;

    if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token_handle) == TRUE)
    {
        TOKEN_PRIVILEGES token{};

        if (::LookupPrivilegeValueA(NULL, name.c_str(), &token.Privileges[0].Luid) == TRUE)
        {
            token.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            result = ::AdjustTokenPrivileges(token_handle, FALSE, &token, 0, NULL, NULL) == TRUE;
        }

        CloseHandle(token_handle);
    }

    return result;
}

namespace mangapp
{
    class watcher_windows : watcher
    {
    public:
        struct windows_context
        {
            OVERLAPPED overlapped;
            size_t key;
            std::wstring path;
            HANDLE handle;
            uint8_t buffer[4096];
            watcher::on_file_event on_event;
            watcher::on_directory_fail_event on_directory_fail;
            watcher * watcher_ptr;
        };

        watcher_windows(on_file_event on_path_change, on_file_event on_file_change, on_directory_fail_event on_directory_fail) :
            watcher(on_path_change, on_file_change, on_directory_fail)
        {
            enable_privilege(SE_BACKUP_NAME);
            enable_privilege(SE_RESTORE_NAME);
        }

        virtual ~watcher_windows()
        {

        }

        virtual void start() final
        {
            m_continue_running = true;
            m_thread = std::thread([this]()
            {
                while (m_continue_running == true || m_handles.size() > 0)
                {
                    m_io_service.run();
                    // Set the thread to an alertable state to process APCs
                    ::SleepEx(10, true);
                }

                // If execution reaches this point, the thread is shutting down.
                // m_handles should be reset in case 'start' is called again
                m_handles.clear();
            });
        }

        virtual void stop() final
        {
            m_continue_running = false;

            // Cancel any pending I/O. The completion routine will take care of cleaning up resources
            for (auto const & kv : m_handles)
            {
                CancelIo(kv.second);
            }
        }

        virtual void add_path(std::wstring const & path) final
        {
            // The calling thread must be the one that goes into an alertable state
            // Therefore, we must use boost::asio::io_service to have the function executed in the context of the watcher thread
            m_io_service.post([this, path]()
            {
                HANDLE handle = ::CreateFileW(path.c_str(),
                                              FILE_LIST_DIRECTORY,
                                              FILE_SHARE_READ,
                                              NULL,
                                              OPEN_EXISTING,
                                              FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                              NULL);
                if (handle != INVALID_HANDLE_VALUE)
                {
                    windows_context * context_ptr = new windows_context{};
                    if (context_ptr != nullptr)
                    {
                        context_ptr->path = path;
                        context_ptr->handle = handle;
                        context_ptr->on_event = m_on_path_change;
                        context_ptr->on_directory_fail = m_on_directory_fail;
                        context_ptr->watcher_ptr = this;
                        BOOL result = ::ReadDirectoryChangesW(handle,
                                                              &context_ptr->buffer[0],
                                                              sizeof(context_ptr->buffer),
                                                              TRUE,
                                                              FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
                                                              NULL,
                                                              &context_ptr->overlapped,
                                                              &file_io_routine);
                        if (result == TRUE)
                        {
                            std::lock_guard<std::mutex> lock(m_mutex);
                            m_handles.emplace(boost::hash<std::wstring>()(path), handle);

                            return;
                        }
                        else
                        {
                            console_write_line("watcher::add_path", "Call to ReadDirectoryChangesW failed. Error code: " + std::to_string(GetLastError()));
                        }
                    }

                    delete context_ptr;
                    ::CloseHandle(handle);
                }
                else
                {
                    console_write_line("watcher::add_path", "Call to CreateFileW failed. Error code: " + std::to_string(GetLastError()));
                }
            });
        }

        virtual void add_file(std::wstring const & file_path) final
        {

        }

        virtual void remove_path(std::wstring const & path) final
        {
			std::lock_guard<std::mutex> lock(m_mutex);
			
            auto iterator = m_handles.find(boost::hash<std::wstring>()(path));
            if (iterator != m_handles.cend())
            {
                CancelIo(iterator->second);
                m_handles.erase(iterator);
            }
        }

        virtual void remove_file(std::wstring const & file_path) final
        {

        }

    protected:
    private:
        std::unordered_map<size_t, HANDLE> m_handles;

        static void CALLBACK file_io_routine(DWORD error_code, DWORD bytes_transferred, OVERLAPPED * overlapped)
        {
            // Our context starts with the OVERLAPPED structure
            windows_context * context_ptr = reinterpret_cast<windows_context *>(overlapped);
            if (context_ptr == nullptr)
                return;

            // Filter out any errors
            if (error_code == ERROR_SUCCESS)
            {
                DWORD offset = 0;
                FILE_NOTIFY_INFORMATION * info_ptr = nullptr;

                do
                {
                    info_ptr = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(&context_ptr->buffer[offset]);

                    std::wstring name(&info_ptr->FileName[0], &info_ptr->FileName[info_ptr->FileNameLength / 2]);
                    std::replace(name.begin(), name.end(), '\\', '/');

                    std::wstring path(context_ptr->path);
                    // path = H:/Manga/Manga/
                    // name = Name/Example/File.zip
                    size_t dir_name_end_pos = name.find('/');
                    if (dir_name_end_pos != std::wstring::npos)
                    {
                        // Trim 'Name' and get its key
                        std::wstring dir_name(name.substr(0, dir_name_end_pos));
                        context_ptr->key = boost::hash<std::wstring>()(dir_name);
                        name.erase(0, ++dir_name_end_pos);

                        path += dir_name + L"/";
                        
                        std::wstring middle_path;
                        size_t middle_end_pos = name.rfind('/');
                        if (middle_end_pos != std::wstring::npos)
                        {
                            // Cut 'Example' to context_ptr->path
                            middle_path = name.substr(0, ++middle_end_pos);
                            name.erase(0, middle_end_pos);
                            path += middle_path;
                        }
                        // And we're left with 'File.zip'
                    }
                    else
                    {
                        context_ptr->key = boost::hash<std::wstring>()(name);
                    }

                    // Using boost::filesystem::is_directory or ::GetFileAttributesW can fail with INVALID_FILE_ATTRIBUTES
                    // Most likely because the object could no longer exist by the time this code is executed
                    // Checking for the presence of a file extension would not work because a folder could be named "folder.name.hi"
                    boost::system::error_code ec;
                    bool is_directory = boost::filesystem::is_directory(path + name, ec);
                    // Did an error occur?
                    if (is_directory == false && ec != 0)
                        // Yes, check the library if it's a directory
                        is_directory = context_ptr->on_directory_fail(name);
                        
                    switch (info_ptr->Action)
                    {
                    case FILE_ACTION_ADDED:
                    case FILE_ACTION_RENAMED_NEW_NAME:
                    {
                        if (context_ptr->on_event != nullptr)
                        {
                            // Signal that a new object was created/added
                            context_ptr->on_event(path, name, is_directory, false, context_ptr->key);
                        }

                        break;
                    }

                    case FILE_ACTION_REMOVED:
                    case FILE_ACTION_RENAMED_OLD_NAME:
                    {
                        if (context_ptr->on_event != nullptr)
                        {
                            // Signal that an existing object was removed/deleted
                            context_ptr->on_event(path, name, is_directory, true, context_ptr->key);
                        }

                        break;
                    }

                    default:
                        // Do nothing for events we do not handle
                        break;
                    }

                    offset += info_ptr->NextEntryOffset;
                } while (info_ptr->NextEntryOffset);

                // Query for more changes
                BOOL result = ::ReadDirectoryChangesW(context_ptr->handle,
                                                      &context_ptr->buffer[0],
                                                      sizeof(context_ptr->buffer),
                                                      FALSE,
                                                      FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
                                                      NULL,
                                                      &context_ptr->overlapped,
                                                      &file_io_routine);
            }
            else
            {
                CloseHandle(context_ptr->handle);
                delete context_ptr;
            }
        }
    };
}

#endif