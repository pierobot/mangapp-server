#ifndef MANGAPP_LIBRARY_HPP
#define MANGAPP_LIBRARY_HPP

#include "directory_entry.hpp"
#include "file_enumeration.hpp"

#include <cstdint>
#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include <utf8/utf8.h>

namespace base
{
    template<class DirectoryEntryType>
    class library
    {
    public:
        typedef size_t key_type;
        typedef DirectoryEntryType entry_type;

        typedef std::unordered_map<key_type, entry_type> library_map_t;
        typedef typename library_map_t::const_iterator const_iterator;

        /**
        *  Constructor for the library. Multiple paths can be parsed.
        *
        *  @param a vector containing the file path(s)
        */
        library(std::vector<std::wstring> const & library_paths) :
            m_entries()
        {
            // Iterate through the libraries
            for (auto const & library_path : library_paths)
            {
                enumerate_files(library_path, file_search_flags::FlagDirectory, false,
                    [this](std::wstring const & path, file_search_flags flag)
                {
                    auto path_end_pos = path.rfind(L"/");

                    std::wstring name_str = path.substr(path_end_pos + 1);
                    std::wstring path_str(path, 0, path_end_pos + 1);

                    auto key = boost::hash<std::wstring>()(name_str);
                    m_entries.emplace(key, entry_type(path_str, name_str, key));
                });
            }
        }

        virtual ~library()
        {
        }

        const_iterator cbegin() const
        {
            return m_entries.cbegin();
        }

        const_iterator cend() const
        {
            return m_entries.cend();
        }

        const_iterator find(size_t key) const
        {
            return m_entries.find(key);
        }

        size_t size() const
        {
            return m_entries.size();
        }
    protected:
    private:
        library_map_t m_entries;
    };
}

#endif
