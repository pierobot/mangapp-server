#include "manga_entry.hpp"

#include <utf8/utf8.h>

using namespace mangapp;

manga_entry::manga_entry(std::wstring const & path, std::wstring const & name, size_t key) : 
    base::file_entry(path, name, key)
{
    std::string utf8_name;
    utf8::utf16to8(name.cbegin(), name.cend(), std::back_inserter(utf8_name));

    m_json = json11::Json::array{ std::to_string(key), utf8_name };
}

manga_entry::~manga_entry()
{
}

json11::Json const & manga_entry::to_json() const
{
    return m_json;
}