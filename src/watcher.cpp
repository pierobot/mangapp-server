#include "watcher.hpp"

#if defined(_WIN32)
#   include "watcher_win.hpp"
#elif defined(__linux__)
#   include "watcher_linux.hpp"
#endif

namespace mangapp
{
    watcher::watcher(on_file_event on_path_change, on_file_event on_file_change) :
        m_on_path_change(on_path_change),
        m_on_file_change(on_file_change),
        m_io_service(),
        m_continue_running(true)
    {
    }

    watcher::~watcher()
    {
        stop();
    }

    void watcher::stop()
    {
        m_continue_running = false;
    }

    std::unique_ptr<watcher> watcher::create(on_file_event on_path_change, on_file_event on_file_change)
    {
#if defined(_WIN32)
        return std::unique_ptr<watcher>(new watcher_windows(on_path_change, on_file_change));
#elif defined(__linux)
#endif
        return nullptr;
    }
}