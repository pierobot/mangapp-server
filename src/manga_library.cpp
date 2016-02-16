#include "manga_library.hpp"
#include "http_utility.hpp"
#include "mangaupdates_parser.hpp"
#include "sort.hpp"

#include "archive_zip.hpp"
#include "archive_rar.hpp"
#include "archive_7z.hpp"

#include <iostream>

#include <utf8/utf8.h>

#include <boost/functional/hash.hpp>

namespace mangapp
{
    manga_library::manga_library(std::vector<std::wstring> const & library_paths) :
        library<manga_directory>(library_paths)
    {
    }

    manga_library::manga_library(std::vector<std::string> const & library_paths) : 
        library<manga_directory>(library_paths)
    {
    }

    manga_library::manga_library(json11::Json const & library_paths) : 
        library<manga_directory>(library_paths)
    {
    }

    manga_library::~manga_library()
    {
    }

    json11::Json const manga_library::get_list() const
    {
        std::vector<std::reference_wrapper<json11::Json const>> entries;

        std::for_each(library::cbegin(), library::cend(),
            [&entries](library_map_t::value_type const & entry)
        {
            entries.push_back(entry.second.to_json());
        });

        std::sort(entries.begin(), entries.end(),
            [](json11::Json const & left, json11::Json const & right) -> bool
        {
            auto const & left_str = left.array_items()[1].string_value();
            auto const & right_str = right.array_items()[1].string_value();

            return std::locale()(left_str, right_str);
        });

        return json11::Json(entries);
    }

    json11::Json const manga_library::get_files(size_t key) const
    {
        auto manga_iterator = library::find(key);
        if (manga_iterator != library::cend())
        {
            std::vector<std::reference_wrapper<json11::Json const>> entries;

            std::for_each(manga_iterator->second.cbegin(), manga_iterator->second.cend(),
                [&entries](manga_directory::map_type::value_type const & value) -> void
            {
                auto const & extension = value.second.get_extension();
                if (is_in_container(g_archive_extensions, extension) == true)
                {
                    entries.push_back(value.second.to_json());
                }
            });

            std::sort(entries.begin(), entries.end(),
                [](json11::Json const & left, json11::Json const & right) -> bool
            {
                auto const & left_str = left.array_items()[1].string_value();
                auto const & right_str = right.array_items()[1].string_value();

                return sort::numerical_regex_less()(left_str, right_str);
            });

            return json11::Json(entries);
        }

        return nullptr;
    }

    std::string manga_library::get_thumbnail(key_type key, bool folder, uint32_t index /*= 0*/) const
    {
        std::string thumb_data;

        auto manga_iterator = library::find(key);
        // Does the specified key exist in our map of manga?
        if (manga_iterator != library::cend())
        {
            // Yes, do we want the folder's thumbnail? 
            if (folder == true)
            {
                // Yes, get the contents of "folder.jpg"
                //std::wstring folder_thumb_path = manga_iterator->second.get_path() +
                //                                 manga_iterator->second.get_name() + L"/folder.jpg";
                size_t thumb_key = boost::hash<std::wstring>()(L"folder.jpg");
                auto thumb_iterator = manga_iterator->second.find(thumb_key);
                if (thumb_iterator != manga_iterator->second.cend())
                {
                    thumb_data = thumb_iterator->second.contents();
                }
                //thumb_data = read_file(key, folder_thumb_path);
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

    void manga_library::search_mangaupdates_for(std::string const & name, std::function<void(json11::Json)> on_event)
    {
        std::string search_url = "/series.html?page=1&stype=title&search=" + http_utility::encode_uri(http_utility::encode_ncr(name));

        auto on_error = [on_event](std::string const & error_msg)
        {
            on_event(json11::Json({ "Result", "Failure: " + error_msg }));
        };

        m_http_helper.http_get_async("www.mangaupdates.com", search_url,
            [this, name, on_event, on_error](std::string const & contents)
        {
            std::string series_id = mangaupdates::get_id(contents, name);
            if (series_id == "")
                on_error(std::string("Unable to find manga with name: ") + name);

            std::string series_url = "/series.html?id=" + series_id;

            m_http_helper.http_get_async("www.mangaupdates.com", series_url,
                [series_id, on_event](std::string const & contents)
            {
                auto description = mangaupdates::get_description(contents);
                auto associated_names = mangaupdates::get_associated_names(contents);
                auto genres = mangaupdates::get_genres(contents);
                auto authors = mangaupdates::get_authors(contents);
                auto artists = mangaupdates::get_artists(contents);
                auto year = mangaupdates::get_year(contents);

                on_event(json11::Json::object{
                    { "Result", "Success" },
                    { "Id", series_id },
                    { "AssociatedNames", associated_names },
                    { "Genres", genres },
                    { "Authors", authors },
                    { "Artists", artists },
                    { "Year", year } });
            }, on_error);
        }, on_error);
    }

    void manga_library::get_manga_details(size_t key, std::function<void(json11::Json)> on_event)
    {
        // Is the key valid?
        auto manga_iterator = library::find(key);
        if (manga_iterator != library::cend())
        {
            // Yes, proceed
            std::wstring const & wname = manga_iterator->second.get_name();
            std::string mbname;

            // Encode the name to UTF-8
            utf8::utf16to8(wname.cbegin(), wname.cend(), std::back_inserter(mbname));

            search_mangaupdates_for(mbname, on_event);
        }
    }

    std::string manga_library::get_image(key_type key, key_type file_key, size_t index) const
    {
        std::string image_contents;

        // Is the manga key valid?
        auto manga_iterator = library::find(key);
        if (manga_iterator != library::cend())
        {
            // Yes. Is the file archive key valid?
            auto archive_iterator = manga_iterator->second.find(file_key);
            if (archive_iterator != manga_iterator->second.cend())
            {
                std::unique_ptr<archive> archive_ptr(nullptr);

                auto file_path = archive_iterator->second.get_path() + archive_iterator->second.get_name();
                auto const extension = archive_iterator->second.get_extension();
                if (extension == L".zip" || extension == L".cbz")
                {
                    archive_ptr = archive::open<archive_zip>(file_path);
                }
                else if (extension == L".rar" || extension == L".cbr")
                {
                    archive_ptr = archive::open<archive_rar>(file_path);
                }
                else if (extension == L".7z" || extension == L".cb7")
                {
                    archive_ptr = archive::open<archive_7z>(file_path);
                }

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
}