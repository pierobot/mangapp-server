#include "manga_entry.hpp"
#include "utf8.hpp"

namespace mangapp
{
    manga_entry::manga_entry(std::wstring const & path, std::wstring const & name, size_t key) :
        base::file_entry(path, name, key)
    {
    }

    manga_entry::~manga_entry()
    {
    }
}
