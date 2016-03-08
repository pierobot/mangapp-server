#include "manga_directory_entry.hpp"

namespace mangapp
{
    manga_directory::manga_directory(std::wstring const & path, std::wstring const & name, size_t key) :
        base::directory_entry<manga_entry>(path, name, key)
    {
    }

    manga_directory::~manga_directory()
    {
    }
}
