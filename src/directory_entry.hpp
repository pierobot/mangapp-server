#ifndef DIRECTORY_ENTRY_HPP
#define DIRECTORY_ENTRY_HPP

#include "directory_file_entry.hpp"
#include "file_enumeration.hpp"
#include "http_utility.hpp"
#include "utf8.hpp"

#include <unordered_map>
#include <memory>
#include <string>

#include <boost/functional/hash.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>

namespace base
{
    template<class FileEntryType>
    class directory_entry
    {
    public:
        typedef FileEntryType file_entry_type;
        typedef std::unordered_map<size_t, file_entry_type> map_type;

        typedef typename map_type::const_iterator const_iterator;

        directory_entry(std::wstring const & path, std::wstring const & name, size_t key) :
            m_path(path),
            m_name(name),
            m_key(key)
        {
            enumerate_files(path + name, file_search_flags::FlagFile, false,
                [this, &path](std::wstring const & full_path, file_search_flags flag)
            {
                auto path_end_pos = full_path.rfind(L"/");
                
                std::wstring name_str = full_path.substr(path_end_pos + 1);
                std::wstring path_str(full_path, 0, path_end_pos + 1);

                auto key = boost::hash<std::wstring>()(name_str);
                m_files.emplace(key, file_entry_type(path_str, name_str, key));
            });
        }


        virtual ~directory_entry()
        {
        }

        const_iterator cbegin() const
        {
            return m_files.cbegin();
        }

        const_iterator cend() const
        {
            return m_files.cend();
        }

        const_iterator find(size_t key) const
        {
            return m_files.find(key);
        }

        std::wstring const & get_path() const
        {
            return m_path;
        }

        std::wstring const & get_name() const
        {
            return m_name;
        }

        size_t const get_key() const
        {
            return m_key;
        }
    protected:
        map_type m_files;
    private:
        std::wstring const m_path;
        std::wstring const m_name;
        size_t const m_key;
    };

}

#endif