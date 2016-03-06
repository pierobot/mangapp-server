#include "manga_library.hpp"
#include "http_utility.hpp"
#include "mangaupdates_parser.hpp"

#include <iostream>

#include <json11/json11.hpp>

#include <utf8/utf8.h>

#include <boost/functional/hash.hpp>

#include <mstch/mstch.hpp>

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

    //json11::Json const manga_library::get_list() const
    //{
    //    std::vector<std::reference_wrapper<json11::Json const>> entries;

    //    std::for_each(library::cbegin(), library::cend(),
    //        [&entries](library_map_t::value_type const & entry)
    //    {
    //        entries.push_back(entry.second.to_json());
    //    });

    //    std::sort(entries.begin(), entries.end(),
    //        [](json11::Json const & left, json11::Json const & right) -> bool
    //    {
    //        auto const & left_str = left["name"].string_value();
    //        auto const & right_str = right["name"].string_value();

    //        return std::locale()(left_str, right_str);
    //    });

    //    return json11::Json(entries);
    //}

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
                size_t thumb_key = boost::hash<std::wstring>()(L"folder.jpg");
                auto thumb_iterator = manga_iterator->second.find(thumb_key);
                if (thumb_iterator != manga_iterator->second.cend())
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

    void manga_library::search_mangaupdates_for(std::string const & name, 
                                                std::function<void(mstch::map&&, bool)> on_event)
    {
        std::string search_url = "/series.html?page=1&stype=title&search=" + http_utility::encode_uri(http_utility::encode_ncr(name));

        auto on_error = [on_event](std::string const & error_msg)
        {
            on_event(mstch::map({}), false);
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
                auto associated_names = mangaupdates::get_associated_names(contents);
                auto description = mangaupdates::get_description(contents);
                auto genres = mangaupdates::get_genres(contents);
                auto authors = mangaupdates::get_authors(contents);
                auto artists = mangaupdates::get_artists(contents);
                auto year = mangaupdates::get_year(contents);

                mstch::array names_array;
                for (auto const & name : associated_names)
                {
                    names_array.emplace_back(mstch::map{
                        { "name", name }
                    });
                }

                mstch::array authors_array;
                for (auto const & author : authors)
                {
                    authors_array.emplace_back(mstch::map{
                        { "author", author }
                    });
                }

                mstch::array artists_array;
                for (auto const & artist : artists)
                {
                    artists_array.emplace_back(mstch::map{
                        { "artist", artist }
                    });
                }

                mstch::array genres_array;
                for (auto const & genre : genres)
                {
                    genres_array.emplace_back(mstch::map{
                        { "genre", genre }
                    });
                }

                mstch::map context{
                    { "names-list", names_array },
                    /*{ "description", description },*/
                    { "authors-list", authors_array},
                    { "artists-list", artists_array },
                    { "genres-list", genres_array },
                    { "year", year }
                };

                on_event(std::move(context), true);

            }, on_error);
        }, on_error);
    }

    void manga_library::get_manga_details(key_type key, std::function<void(mstch::map&&, bool)> on_event)
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
                auto file_path = archive_iterator->second.get_path() + archive_iterator->second.get_name();

                auto archive_ptr = archive::open(file_path);
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