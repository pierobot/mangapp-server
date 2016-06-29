#ifndef WATCHER_WIN_HPP
#define WATCHER_WIN_HPP

#include "watcher.hpp"

#include <Windows.h>

#include <iterator>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <boost/functional/hash.hpp>

extern void console_write_line(std::string const & function_name, std::string const & message);

namespace mangapp
{
    class watcher_windows : watcher
    {
    public:
        struct context
        {
            OVERLAPPED overlapped;
            std::wstring path;
            HANDLE handle;
            uint8_t buffer[1024];
            watcher::on_file_event on_event;
            watcher * watcher_ptr;
        };

        watcher_windows(on_file_event on_path_change, on_file_event on_file_change) :
            watcher(on_path_change, on_file_change)
        {
        }

        virtual ~watcher_windows()
        {

        }

        virtual void start() final
        {
            m_thread = std::thread([this]()
            {
                while (m_continue_running == true)
                {
                    m_io_service.run();
                    // Set the thread to an alertable state to process APCs
                    ::SleepEx(10, true);
                }
            });
        }

        virtual void add_path(std::wstring const & path) final
        {
            // The calling thread must be the one that goes into an alertable state
            // Therefore, we must use boost::asio::io_service to have the function executed in the context of the watcher thread
            m_io_service.post([this, path]()
            {
                HANDLE handle = ::CreateFileW(path.c_str(),
                                              FILE_LIST_DIRECTORY,
                                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                              NULL,
                                              OPEN_EXISTING,
                                              FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                              NULL);
                if (handle != INVALID_HANDLE_VALUE)
                {
                    context * context_ptr = new context{};
                    if (context_ptr != nullptr)
                    {
                        context_ptr->path = path;
                        context_ptr->handle = handle;
                        context_ptr->on_event = m_on_path_change;
                        context_ptr->watcher_ptr = this;
                        BOOL result = ::ReadDirectoryChangesW(handle,
                                                              &context_ptr->buffer[0],
                                                              sizeof(context_ptr->buffer),
                                                              FALSE,
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
                CloseHandle(iterator->second);
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
            context * context_ptr = reinterpret_cast<context *>(overlapped);
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

                    switch (info_ptr->Action)
                    {
                    case FILE_ACTION_ADDED:
                    case FILE_ACTION_RENAMED_NEW_NAME:
                    {
                        if (context_ptr->on_event != nullptr)
                        {
                            // Signal that a new object was created/added
                            context_ptr->on_event(context_ptr->path, name, false);
                        }

                        break;
                    }

                    case FILE_ACTION_REMOVED:
                    case FILE_ACTION_RENAMED_OLD_NAME:
                    {
                        if (context_ptr->on_event != nullptr)
                        {
                            // Signal that an existing object was removed/deleted
                            context_ptr->on_event(context_ptr->path, name, true);
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
                console_write_line("aaaa", "test");
                context_ptr->watcher_ptr->remove_path(context_ptr->path);
                delete context_ptr;
            }
        }
    };
}

#endif