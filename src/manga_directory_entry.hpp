#ifndef MANGAPP_MANGA_DIRECTORY_ENTRY_HPP
#define MANGAPP_MANGA_DIRECTORY_ENTRY_HPP

#include "directory_entry.hpp"
#include "manga_entry.hpp"

#include <string>

namespace mangapp
{
    class manga_directory : public base::directory_entry<manga_entry>
    {
    public:
        manga_directory(std::wstring const & path, std::wstring const & name, size_t key);
        
        virtual ~manga_directory();


    protected:
    private:
    };
}

#endif