#ifndef WATCHER_WIN_HPP
#define WATCHER_WIN_HPP

#include "watcher.hpp"

namespace mangapp
{
    class watcher_linux : watcher
    {
    public:
        watcher_linux(on_file_event on_path_change, on_file_event on_file_change, on_directory_fail_event on_directory_fail) :
            watcher(on_path_change, on_file_change, on_directory_fail)
        {
        }

        virtual ~watcher_linux()
        {
        }

        virtual void start() final
        {
        }

        virtual void stop() final
        {
        }

        virtual void add_path(std::wstring const & path) final
        {
        }

        virtual void remove_path(std::wstring const & path) final
        {
        }
    protected:
    private:

    };
}

#endif