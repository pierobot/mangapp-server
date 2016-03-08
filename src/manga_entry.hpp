#ifndef MANGAPP_MANGA_ENTRY_HPP
#define MANGAPP_MANGA_ENTRY_HPP

#include "directory_file_entry.hpp"

namespace mangapp
{
    class manga_entry : public base::file_entry
    {
    public:
        manga_entry(std::wstring const & path, std::wstring const & name, size_t key);
        
        virtual ~manga_entry();
    protected:
    private:
        std::string m_mangaupdates_id;
    };
}

#endif