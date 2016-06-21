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

    void manga_directory::add_possible_matches(std::vector<match_type> && matches)
    {
        std::copy(matches.begin(), matches.end(), std::back_inserter(m_matches));
    }

    manga_directory::match_type const manga_directory::get_best_match() const
    {
        match_type best_match;

        if (m_matches.size() > 0)
            best_match = m_matches.front();

        return best_match;
    }
}
