#ifndef MANGAPP_MANGA_DIRECTORY_ENTRY_HPP
#define MANGAPP_MANGA_DIRECTORY_ENTRY_HPP

#include "directory_entry.hpp"
#include "manga_entry.hpp"
#include "mangaupdates.hpp"

namespace mangapp
{
    class manga_directory : public base::directory_entry<manga_entry>
    {
    public:
        typedef std::pair<float, std::string> match_type;

        manga_directory(std::wstring const & path, std::wstring const & name, size_t key);
        
        virtual ~manga_directory();

        void add_possible_matches(std::vector<match_type> && matches);
        match_type const get_best_match() const;
    protected:
    private:
        std::vector<match_type> m_matches;
    };
}

#endif