#ifndef WATCHER_WIN_HPP
#define WATCHER_WIN_HPP

#include "watcher.hpp"

namespace mangapp
{
    class watcher_linux : watcher
    {
    public:
        watcher_linux(on_file_event on_path_change, on_file_event on_file_change) :
            watcher(on_path_change, on_file_change)
        {
        }

        virtual ~watcher_linux()
        {
        }

        virtual void add_path(std::string const & path) final
        {

        }

        virtual void add_file(std::string const & file_path) final
        {

        }

        virtual void remove_path(std::string const & path) final
        {

        }

        virtual void remove_file(std::string const & file_path) final
        {

        }

    protected:
    private:

    };
}

#endif