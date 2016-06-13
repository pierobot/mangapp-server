#ifndef MANGAPP_LIBRARY_HPP
#define MANGAPP_LIBRARY_HPP

#include "archive.hpp"
#include "directory_entry.hpp"
#include "file_enumeration.hpp"
#include "sort.hpp"
#include "users.hpp"
#include "utf8.hpp"
#include "image.hpp"

#include <unordered_map>

#include <json11/json11.hpp>

#include <mstch/mstch.hpp>

#include <sqlite3pp/headeronly_src/sqlite3pp.h>


namespace base
{
    namespace
    {
        std::vector<std::wstring> utf8_paths_to_utf16(std::vector<std::string> const & library_paths)
        {
            std::vector<std::wstring> utf16_paths;

            for (auto const & path : library_paths)
            {
                std::wstring utf16_str = to_utf16(path);

                utf16_paths.push_back(utf16_str);
            }

            return utf16_paths;
        }

        std::vector<std::wstring> json_to_utf16_vector(json11::Json const & library_paths)
        {
            std::vector<std::wstring> utf16_paths;

            for (auto const & element : library_paths.array_items())
            {
                json11::Json const & object = element.object_items();
                std::string utf8_str(object["path"].string_value());
                std::wstring utf16_str = to_utf16(utf8_str);

                utf16_paths.emplace_back(utf16_str);
            }

            return utf16_paths;
        }
    }

    template<class DirectoryEntryType>
    class library
    {
    public:
        typedef size_t key_type;
        typedef DirectoryEntryType directory_entry_type;
        typedef typename DirectoryEntryType::file_entry_type file_entry_type;

        typedef std::unordered_map<key_type, directory_entry_type> library_map_type;
        typedef typename library_map_type::iterator iterator;
        typedef typename library_map_type::const_iterator const_iterator;

        typedef std::function<void(sqlite3pp::query::iterator, sqlite3pp::query::iterator)> on_query_function;

        /**
        *  Constructor for the library. Multiple paths can be parsed.
        *
        *  @param library_paths a vector containing the file path(s)
        */
        library(std::vector<std::wstring> const & library_paths, mangapp::users & users, std::string const & database_file) :
            m_entries(),
            m_users(users),
            m_database_file(database_file)
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
                    m_entries.emplace(key, directory_entry_type(path_str, name_str, key));
                });
            }
        }

        library(std::vector<std::string> const & library_paths, mangapp::users & users, std::string const & database_file) :
            library(utf8_paths_to_utf16(library_paths), users, database_file)
        {
        }

        library(json11::Json const & settings_json, mangapp::users & users, std::string const & database_file) :
            library(json_to_utf16_vector(settings_json["manga"]), users, database_file)
        {
        }

        virtual ~library()
        {
        }

        iterator begin()
        {
            return m_entries.begin();
        }

        iterator end()
        {
            return m_entries.end();
        }

        const_iterator cbegin() const
        {
            return m_entries.cbegin();
        }

        const_iterator cend() const
        {
            return m_entries.cend();
        }

        iterator find(key_type key)
        {
            return m_entries.find(key);
        }

        const_iterator find(key_type key) const
        {
            return m_entries.find(key);
        }

        key_type size() const
        {
            return m_entries.size();
        }

        mstch::map const get_list_context(std::string const & session_id) const
        {
            mstch::array library_array;

            // Enumerate all the entries the user can access
            std::vector<std::reference_wrapper<directory_entry_type const>> ordered;
            for (auto const & pair : m_entries)
            {
                size_t path_key = boost::hash<std::wstring>()(pair.second.get_path());
                if (m_users.can_access(session_id, path_key) == true)
                    ordered.push_back(std::cref(pair.second));
            }

            // Collate the entries
            std::sort(ordered.begin(), ordered.end(),
                [](std::reference_wrapper<directory_entry_type const> const & left,
                   std::reference_wrapper<directory_entry_type const> const & right) -> bool
            {
                return std::locale()(left.get().get_name(), right.get().get_name());
            });

            // Construct the array of manga/comics
            for (auto const & entry : ordered)
            {
                std::wstring utf16_name(entry.get().get_name());
                std::string utf8_name = to_utf8(utf16_name);

                library_array.emplace_back(mstch::map{
                    { "key",  std::to_string(entry.get().get_key()) },
                    { "name", utf8_name }
                });
            }

            return mstch::map({
                { "library-list", library_array }
            });
        }

        mstch::map get_files_context(std::string const & session_id, key_type key) const
        {
            auto const manga_comic_iterator = library::find(key);
            if (manga_comic_iterator != library::cend())
            {
                // Firstly, ensure the user has access
                size_t path_key = boost::hash<std::wstring>()(manga_comic_iterator->second.get_path());
                if (m_users.can_access(session_id, path_key) == false)
                    return mstch::map();

                // Enumerate all the files from the directory_entry
                std::vector<std::reference_wrapper<library::file_entry_type const>> ordered;

                std::for_each(manga_comic_iterator->second.cbegin(), manga_comic_iterator->second.cend(),
                    [&ordered](typename library::directory_entry_type::map_type::value_type const & entry)
                {
                    ordered.push_back(std::cref(entry.second));
                });

                // Sort the filenames based on a regex rule
                std::sort(ordered.begin(), ordered.end(),
                    [](std::reference_wrapper<library::file_entry_type const> const & left,
                       std::reference_wrapper<library::file_entry_type const> const & right)
                {
                    return sort::numerical_wregex_less()(left.get().get_name(), right.get().get_name());
                });

                mstch::array files_array{};
                
                // Construct and populate the array of files
                std::for_each(ordered.cbegin(), ordered.cend(),
                    [&files_array, key](std::reference_wrapper<library::file_entry_type const> const & value) -> void
                {
                    auto const & entry = value.get();

                    // Filter out any files that are not archives
                    if (is_archive_extension(entry.get_extension()) == true)
                    {
                        std::wstring const & utf16_name(entry.get_name());
                        std::string utf8_name = to_utf8(utf16_name);

                        files_array.emplace_back(mstch::map({
                            { "key", std::to_string(key) },
                            { "file-key", std::to_string(entry.get_key()) },
                            { "file-name", utf8_name }
                        }));
                    }
                });

                std::wstring const & utf16_name(manga_comic_iterator->second.get_name());
                std::string utf8_name = to_utf8(utf16_name);

                return mstch::map({
                    { "key", std::to_string(key) },
                    { "name", std::move(utf8_name) },
                    { "file-count", std::to_string(files_array.size()) },
                    { "files-list", std::move(files_array) },
                });
            }

            return mstch::map();
        }

        mstch::map const get_reader_context(std::string const & session_id, key_type key, key_type file_key) const
        {
            auto const entry_iterator = find(key);
            if (entry_iterator != cend())
            {
                // Ensure the user has access
                size_t path_key = boost::hash<std::wstring>()(entry_iterator->second.get_path());
                if (m_users.can_access(session_id, path_key) == false)
                    return mstch::map();

                // Find the requested archive based from its key
                auto archive_iterator = entry_iterator->second.find(file_key);
                // Was it found?
                if (archive_iterator != entry_iterator->second.cend())
                {
                    // Yes, get its path and open the archive
                    auto file_path = archive_iterator->second.get_path() + archive_iterator->second.get_name();
                    auto archive_ptr = mangapp::archive::open(file_path);
                    // Ensure we have a valid pointer
                    if (archive_ptr != nullptr)
                    {
                        mstch::array image_array;
                        mstch::array image_dropdown;
                        uint32_t image_count = 0;

                        // Populate the array of images
                        for (auto const & image_entry : *archive_ptr)
                        {
                            // Filter out any files that are not images
                            if (is_image_extension(image_entry->extension()) == true)
                            {
                                image_array.emplace_back(mstch::map({
                                    { "key", std::to_string(key) },
                                    { "file-key", std::to_string(file_key) },
                                    { "index", std::to_string(image_count) }
                                }));

                                image_dropdown.emplace_back(mstch::map({
                                    { "image-index", std::to_string(++image_count) }
                                }));
                            }
                        }

                        std::wstring const & utf16_name(entry_iterator->second.get_name());
                        std::string utf8_name = to_utf8(utf16_name);

                        return mstch::map({
                            { "name", utf8_name },
                            { "image-count", std::to_string(image_count) },
                            { "image-dropdown", image_dropdown },
                            { "image-list", image_array }
                        });
                    }
                }
            }

            return mstch::map();
        }

        /**
        *   Reads the thumbnail of a manga/comic from its folder or from an archive.
        *   Retrieval from an archive is NOT implemented yet.
        *
        *   @param key the key of the manga/comic
        *   @param index index of the archive in the folder
        *   @return a string containing the thumbnail's contents
        */
        std::string get_thumbnail(std::string const & session_id, key_type key, uint32_t index = 0) const
        {
            std::string thumb_data;

            auto const manga_comic_iterator = find(key);
            // Does the specified key exist in our map?
            if (manga_comic_iterator != cend())
            {
                // Look for an existing thumbnail
                directory_entry_type const & dir_entry = manga_comic_iterator->second;
                size_t thumb_key_1 = boost::hash<std::wstring>()(L"folder.jpg");
                size_t thumb_key_2 = boost::hash<std::wstring>()(L"folder.jpeg");

                auto thumb_iterator_1 = dir_entry.find(thumb_key_1);
                auto thumb_iterator_2 = dir_entry.find(thumb_key_2);
                if (thumb_iterator_1 != dir_entry.cend())
                {
                    thumb_data = thumb_iterator_1->second.contents();
                }
                else if (thumb_iterator_2 != dir_entry.cend())
                {
                    thumb_data = thumb_iterator_2->second.contents();
                }
                else
                {
                    // No existing thumbnail - get one
                    if (dir_entry.size() > 0)
                    {
                        file_entry_type const & file_entry = dir_entry.cbegin()->second;
                        auto archive_key = file_entry.get_key();
                        auto image = get_image(session_id, key, archive_key, 0);
                        if (image.empty() == false)
                        {
                            mangapp::image img(image);
                            img.resize(128, 180);
                            thumb_data = img.contents(".jpeg");
                        }
                    }
                }
            }

            return thumb_data;
        }

        /**
        *   Gets the details of a manga/comic
        *
        *   @param key the key of the manga.comic
        *   @param on_event a function object that will be called on success or failure
        */
        void get_details(key_type key, std::function<void(mstch::map&&, bool)> on_event)
        {
            // Is the key valid?
            auto const manga_comic_iterator = library::find(key);
            if (manga_comic_iterator != library::cend())
            {
                // Yes, proceed
                search_online_source(manga_comic_iterator->second, on_event);
            }
        }

        /**
        *   Gets the desired image from an archive.
        *
        *   @param key the key of the manga
        *   @param key the key of the archive file
        *   @param index the index of the image
        *   @return a string containing the image's contents
        */
        std::string get_image(std::string const & session_id, key_type key, key_type file_key, size_t index) const
        {
            std::string image_contents;

            // Is the manga key valid?
            auto const manga_comic_iterator = library::find(key);
            if (manga_comic_iterator != library::cend())
            {
                size_t path_key = boost::hash<std::wstring>()(manga_comic_iterator->second.get_path());
                if (m_users.can_access(session_id, path_key) == false)
                    return "";

                // Yes. Is the file archive key valid?
                auto archive_iterator = manga_comic_iterator->second.find(file_key);
                if (archive_iterator != manga_comic_iterator->second.cend())
                {
                    auto file_path = archive_iterator->second.get_path() + archive_iterator->second.get_name();

                    auto archive_ptr = mangapp::archive::open(file_path);
                    // Make sure we have a valid archive pointer before proceeding
                    if (archive_ptr != nullptr)
                    {
                        // Remove any file entries that are not images
                        archive_ptr->filter([key](mangapp::archive::entry_pointer const & entry_ptr) -> bool
                        {
                            bool remove = is_image_extension(entry_ptr->extension()) == false || entry_ptr->is_dir() == true;

                            return remove;
                        });

                        // Sort them
                        std::sort(archive_ptr->begin(), archive_ptr->end(),
                            [](mangapp::archive::entry_pointer const & left_ptr,
                               mangapp::archive::entry_pointer const & right_ptr) -> int
                        {
                            return std::locale()(left_ptr->name(), right_ptr->name());
                        });

                        // Get the contents of the image
                        if (archive_ptr->count() > 0)
                        {
                            auto const & image_ptr = (*archive_ptr)[index];

                            if (image_ptr != nullptr)
                                image_contents = image_ptr->contents();
                        }
                    }
                }
            }

            return image_contents;
        }
    protected:
        virtual void search_online_source(directory_entry_type & entry, std::function<void(mstch::map&&, bool)> on_event)
        {
        }

        void sqlite3_execute(std::string const & command_str, std::map<std::string, std::string> params)
        {
            sqlite3pp::database database(m_database_file.c_str());
            sqlite3pp::command cmd(database, command_str.c_str());
            
            for (auto const & kv : params)
            {
                cmd.bind(kv.first.c_str(), kv.second.c_str(), sqlite3pp::nocopy);
            }

            cmd.execute();
        }

        void sqlite3_query(std::string const & query_str,
                           std::map<std::string, std::string> params,
                           on_query_function on_query) const
        {
            sqlite3pp::database database(m_database_file.c_str());
            sqlite3pp::query query(database, query_str.c_str());
            
            for (auto const & kv : params)
            {
                query.bind(kv.first.c_str(), kv.second.c_str(), sqlite3pp::nocopy);
            }

            on_query(query.begin(), query.end());
        }
    private:
        library_map_type m_entries;
        mangapp::users & m_users;
        std::string m_database_file;
    };
}

#endif
