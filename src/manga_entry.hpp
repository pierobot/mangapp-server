#ifndef MANGAPP_MANGA_ENTRY_HPP
#define MANGAPP_MANGA_ENTRY_HPP

#include "directory_file_entry.hpp"

#include <json11/json11.hpp>

namespace mangapp
{
    class manga_entry : public base::file_entry
    {
    public:
        manga_entry(std::wstring const & path, std::wstring const & name, size_t key);
        
        virtual ~manga_entry();

        json11::Json const & to_json() const;
    protected:
    private:
        json11::Json m_json;
        std::string m_mangaupdates_id;
    };
}

#endif