#ifndef MANGAPP_LIBRARY_HPP
#define MANGAPP_LIBRARY_HPP

#include "archive.hpp"
#include "directory_entry.hpp"
#include "file_enumeration.hpp"
#include "sort.hpp"

#include <cstdint>
#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include <utf8/utf8.h>

#include <json11/json11.hpp>

#include <mstch/mstch.hpp>

namespace base
{
    namespace
    {
        std::vector<std::wstring> utf8_paths_to_utf16(std::vector<std::string> const & library_paths)
        {
            std::vector<std::wstring> utf16_paths;

            for (auto const & path : library_paths)
            {
                std::wstring utf16_str;
                utf8::utf8to16(path.cbegin(), path.cend(), std::back_inserter(utf16_str));

                utf16_paths.push_back(utf16_str);
            }

            return utf16_paths;
        }

        std::vector<std::wstring> json_to_utf16_vector(json11::Json const & library_paths)
        {
            std::vector<std::wstring> utf16_paths;

            for (auto const & element : library_paths.array_items())
            {
                std::string utf8_str(element.string_value());
                std::wstring utf16_str;
                utf8::utf8to16(utf8_str.cbegin(), utf8_str.cend(), std::back_inserter(utf16_str));

                utf16_paths.emplace_back(utf16_str);
            }

            return utf16_paths;
        }

        static std::vector<std::wstring> const g_image_extensions = { L".jpg", L".jpeg", L".png" };

        bool is_image_extension(std::wstring const & extension)
        {
            return std::find(g_image_extensions.cbegin(), g_image_extensions.cend(), extension) != g_image_extensions.cend();
        }
    }

    template<class DirectoryEntryType>
    class library
    {
    public:
        typedef size_t key_type;
        typedef DirectoryEntryType directory_entry_type;
        typedef typename DirectoryEntryType::file_entry_type file_entry_type;

        typedef std::unordered_map<key_type, directory_entry_type> library_map_t;
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
                    m_entries.emplace(key, directory_entry_type(path_str, name_str, key));
                });
            }
        }

        library(std::vector<std::string> const & library_paths) :
            library(utf8_paths_to_utf16(library_paths))
        {
        }

        library(json11::Json const & library_paths) : 
            library(json_to_utf16_vector(library_paths))
        {

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

        const_iterator find(key_type key) const
        {
            return m_entries.find(key);
        }

        key_type size() const
        {
            return m_entries.size();
        }

        mstch::map const get_list_context() const
        {
            mstch::array library_array;

            std::vector<std::reference_wrapper<directory_entry_type const>> ordered;
            for (auto const & pair : m_entries)
            {
                ordered.push_back(std::cref(pair.second));
            }

            std::sort(ordered.begin(), ordered.end(),
                [](std::reference_wrapper<directory_entry_type const> const & left,
                   std::reference_wrapper<directory_entry_type const> const & right) -> bool
            {
                return std::locale()(left.get().get_name(), right.get().get_name());
            });

            for (auto const & entry : ordered)
            {
                std::wstring utf16_name(entry.get().get_name());
                std::string utf8_name;
                utf8::utf16to8(utf16_name.cbegin(), utf16_name.cend(), std::back_inserter(utf8_name));

                library_array.emplace_back(mstch::map{
                    { "key",  std::to_string(entry.get().get_key()) },
                    { "name", utf8_name }
                });
            }

            return mstch::map({
                { "library-list", library_array }
            });
        }

        mstch::map get_files_context(key_type key) const
        {
            auto manga_comic_iterator = library::find(key);
            if (manga_comic_iterator != library::cend())
            {
                std::vector<std::reference_wrapper<library::file_entry_type const>> ordered;

                std::for_each(manga_comic_iterator->second.cbegin(), manga_comic_iterator->second.cend(),
                    [&ordered](library::directory_entry_type::map_type::value_type const & entry)
                {
                    ordered.push_back(std::cref(entry.second));
                });

                std::sort(ordered.begin(), ordered.end(),
                    [](std::reference_wrapper<library::file_entry_type const> const & left,
                        std::reference_wrapper<library::file_entry_type const> const & right)
                {
                    return sort::numerical_wregex_less()(left.get().get_name(), right.get().get_name());
                });

                mstch::array files_array{};

                std::for_each(ordered.cbegin(), ordered.cend(),
                    [&files_array, key](std::reference_wrapper<library::file_entry_type const> const & value) -> void
                {
                    auto const & entry = value.get();

                    if (is_in_container(g_archive_extensions, entry.get_extension()) == true)
                    {
                        std::wstring const & utf16_name(entry.get_name());
                        std::string utf8_name;

                        utf8::utf16to8(utf16_name.cbegin(), utf16_name.cend(), std::back_inserter(utf8_name));

                        files_array.emplace_back(mstch::map({
                            { "key", std::to_string(key) },
                            { "file-key", std::to_string(entry.get_key()) },
                            { "file-name", utf8_name }
                        }));
                    }
                });

                std::wstring const & utf16_name(manga_comic_iterator->second.get_name());
                std::string utf8_name;
                utf8::utf16to8(utf16_name.cbegin(), utf16_name.cend(), std::back_inserter(utf8_name));

                return mstch::map({
                    { "key", std::to_string(key) },
                    { "name", std::move(utf8_name) },
                    { "file-count", std::to_string(files_array.size()) },
                    { "files-list", std::move(files_array) },
                });
            }

            return mstch::map({});
        }

        mstch::map const get_reader_context(key_type key, key_type file_key) const
        {
            auto entry_iterator = find(key);
            if (entry_iterator != cend())
            {
                auto archive_iterator = entry_iterator->second.find(file_key);
                if (archive_iterator != entry_iterator->second.cend())
                {
                    auto file_path = archive_iterator->second.get_path() + archive_iterator->second.get_name();

                    auto archive_ptr = mangapp::archive::open(file_path);
                    if (archive_ptr != nullptr)
                    {
                        mstch::array image_array;
                        uint32_t image_count = 0;
                        for (auto const & image_entry : *archive_ptr)
                        {
                            if (is_image_extension(image_entry->extension()) == false)
                                continue;

                            ++image_count;
                            image_array.emplace_back(mstch::map({
                                { "key", std::to_string(key) },
                                { "file-key", std::to_string(file_key) },
                                { "index", std::to_string(image_entry->index()) }
                            }));
                        }

                        std::wstring const & utf16_name(entry_iterator->second.get_name());
                        std::string utf8_name;
                        utf8::utf16to8(utf16_name.cbegin(), utf16_name.cend(), std::back_inserter(utf8_name));

                        return mstch::map({
                            { "name", utf8_name },
                            { "image-count", std::to_string(image_count) },
                            { "image-list", image_array }
                        });
                    }
                }
            }

            return mstch::map({});
        }

        /**
        *   Reads the thumbnail of a manga/comic from its folder or from an archive.
        *   Retrieval from an archive is NOT implemented yet.
        *
        *   @param key the key of the manga/comic
        *   @param folder indicates whether or not to retrieve from the folder or archive
        *   @param index index of the archive in the folder
        *   @return a string containing the thumbnail's contents
        */
        std::string get_thumbnail(key_type key, bool folder, uint32_t index = 0) const
        {
            std::string thumb_data;

            auto manga_comic_iterator = find(key);
            // Does the specified key exist in our map?
            if (manga_comic_iterator != cend())
            {
                // Yes, do we want the folder's thumbnail? 
                if (folder == true)
                {
                    // Yes, get the contents of "folder.jpg"
                    size_t thumb_key = boost::hash<std::wstring>()(L"folder.jpg");
                    auto thumb_iterator = manga_comic_iterator->second.find(thumb_key);
                    if (thumb_iterator != manga_comic_iterator->second.cend())
                    {
                        thumb_data = thumb_iterator->second.contents();
                    }
                }
                else
                {
                    // No, get the thumbnail from an archive

                    // NOT IMPLEMENTED YET
                    // NEED IMAGE RESIZE LIBRARY
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
            auto manga_comic_iterator = library::find(key);
            if (manga_comic_iterator != library::cend())
            {
                // Yes, proceed
                std::wstring const & wname = manga_comic_iterator->second.get_name();
                std::string mbname;

                // Encode the name to UTF-8
                utf8::utf16to8(wname.cbegin(), wname.cend(), std::back_inserter(mbname));

                search_online_source(mbname, on_event);
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
        std::string get_image(key_type key, key_type file_key, size_t index) const
        {
            std::string image_contents;

            // Is the manga key valid?
            auto manga_comic_iterator = library::find(key);
            if (manga_comic_iterator != library::cend())
            {
                // Yes. Is the file archive key valid?
                auto archive_iterator = manga_comic_iterator->second.find(file_key);
                if (archive_iterator != manga_comic_iterator->second.cend())
                {
                    auto file_path = archive_iterator->second.get_path() + archive_iterator->second.get_name();

                    auto archive_ptr = mangapp::archive::open(file_path);
                    // Make sure we have a valid archive pointer before proceeding
                    if (archive_ptr != nullptr)
                    {
                        // Get the contents of the image
                        auto image_ptr = (*archive_ptr)[index];
                        if (image_ptr != nullptr)
                            image_contents = image_ptr->contents();
                    }
                }
            }

            return image_contents;
        }
    protected:
        /**
        *   Function that asynchronously searches an online source for the supplied name of the manga/comic.
        *   The results are stored in a mstch::map object and passed to the 'on_event' callback.
        *
        *   @param name the name of the manga/comic
        *   @param on_event a callback that handles the mstch::map object
        */
        virtual void search_online_source(std::string const & name, std::function<void(mstch::map&&, bool)> on_event)
        {
        }
    private:
        library_map_t m_entries;
    };
}

#endif
