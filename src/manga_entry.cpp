#include "manga_entry.hpp"
#include "utf8.hpp"

namespace mangapp
{
    manga_entry::manga_entry(std::wstring const & path, std::wstring const & name, size_t key) :
        base::file_entry(path, name, key)
    {
        std::string utf8_name = to_utf8(name);

        m_json = json11::Json::object{ { "key", std::to_string(key) },
        { "name", utf8_name } };
    }

    manga_entry::~manga_entry()
    {
    }

    json11::Json const & manga_entry::to_json() const
    {
        return m_json;
    }
}
