#ifndef MANGAPP_WATCHER_HPP
#define MANGAPP_WATCHER_HPP

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include <boost/asio.hpp>

namespace mangapp
{
    class watcher
    {
    public:
        typedef std::function<void(std::wstring const &, std::wstring const &, bool)> on_file_event;

        watcher(on_file_event on_path_change, on_file_event on_file_change);

        virtual ~watcher();

        virtual void start() = 0;
        void stop();

        virtual void add_path(std::wstring const & path) = 0;
        virtual void remove_path(std::wstring const & path) = 0;

        static std::unique_ptr<watcher> create(on_file_event on_path_change, on_file_event on_file_change);
    protected:
        std::atomic<bool> m_continue_running;
        std::thread m_thread;
        boost::asio::io_service m_io_service;
        std::mutex m_mutex;
        on_file_event m_on_path_change;
        on_file_event m_on_file_change;
    private:
    };
}

#endif
