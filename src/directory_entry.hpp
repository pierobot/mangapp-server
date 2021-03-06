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

static std::vector<std::wstring> const g_image_extensions = { L".jpg", L".jpeg", L".png",
                                                              L".JPG", L".JPEG", L".PNG",
                                                              L".gif", L".GIF" };

static std::vector<std::wstring> const g_archive_extensions = { L".rar", L".cbr", L".RAR", L".CBR",
                                                                L".zip", L".cbz", L".ZIP", L".CBZ",
                                                                L".7z",  L".cb7", L".7Z", L".CB7" };

inline bool is_archive_extension(std::wstring const & extension)
{
    return std::find(g_archive_extensions.cbegin(), g_archive_extensions.cend(), extension) != g_archive_extensions.cend();
}

inline bool is_image_extension(std::wstring const & extension)
{
    return std::find(g_image_extensions.cbegin(), g_image_extensions.cend(), extension) != g_image_extensions.cend();
}

namespace base
{
    template<class FileEntryType>
    class directory_entry
    {
    public:
        typedef FileEntryType file_entry_type;
        typedef std::unordered_map<size_t, file_entry_type> map_type;

        typedef typename map_type::iterator iterator;
        typedef typename map_type::const_iterator const_iterator;

        directory_entry(std::wstring const & path, std::wstring const & name, size_t key) :
            m_path(path),
            m_name(name),
            m_key(key)
        {
            enumerate_files(path + name, file_search_flags::FlagFile, true,
                [this, &path](std::wstring const & file_path, file_search_flags flag)
            {
                auto extension_start_pos = file_path.rfind(L'.');
                if (extension_start_pos != std::string::npos)
                {
                    auto extension_str = file_path.substr(extension_start_pos);
                    // We only want archives and images
                    if (is_archive_extension(extension_str) == true ||
                        is_image_extension(extension_str) == true)
                    {
                        auto path_end_pos = file_path.rfind(L"/");

                        std::wstring name_str = file_path.substr(path_end_pos + 1);
                        std::wstring path_str(file_path, 0, path_end_pos + 1);

                        auto key = boost::hash<std::wstring>()(name_str);
                        m_files.emplace(key, file_entry_type(path_str, name_str, key));
                    }
                }
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

        size_t const size() const
        {
            return m_files.size();
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

        void add_file(std::wstring const & path, std::wstring const & name)
        {
            boost::filesystem::path bpath(name);
            auto const & extension = bpath.extension().generic_wstring();
            
            if (is_image_extension(extension) == true ||
                is_archive_extension(extension) == true)
            {
                auto key = boost::hash<std::wstring>()(name);
                m_files.emplace(key, file_entry_type(path, name, key));
            }
        }

        void remove_file(std::wstring const & name)
        {
            auto key = boost::hash<std::wstring>()(name);
            auto file_iterator = m_files.find(key);

            if (file_iterator != m_files.cend())
            {
                m_files.erase(file_iterator);
            }
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