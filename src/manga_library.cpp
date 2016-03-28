#include "manga_library.hpp"
#include "http_utility.hpp"
#include "mangaupdates_parser.hpp"
#include "utf8.hpp"

#include <fstream>
#include <mutex>

#include <json11/json11.hpp>

#include <boost/functional/hash.hpp>

#include <mstch/mstch.hpp>

extern std::mutex g_mutex_cout;

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

    void manga_library::search_online_source(key_type key,
                                             std::string const & name,
                                             std::function<void(mstch::map&&, bool)> on_event)
    {
        std::string search_url = "/series.html?page=1&stype=title&search=" + http_utility::encode_uri(http_utility::encode_ncr(name));

        auto on_error = [on_event](std::string const & error_msg)
        {
            on_event(mstch::map({}), false);
        };

        m_http_helper.http_get_async("www.mangaupdates.com", search_url,
            [this, key, name, on_event, on_error](std::string const & contents)
        {
            std::string series_id = mangaupdates::get_id(contents, name);
            if (series_id == "")
                on_error(std::string("Unable to find manga with name: ") + name);

            std::string series_url = "/series.html?id=" + series_id;

            m_http_helper.http_get_async("www.mangaupdates.com", series_url,
                [key, series_id, on_event](std::string const & contents)
            {
                auto associated_names = mangaupdates::get_associated_names(contents);
                auto description = mangaupdates::get_description(contents);
                auto genres = mangaupdates::get_genres(contents);
                auto authors = mangaupdates::get_authors(contents);
                auto artists = mangaupdates::get_artists(contents);
                auto year = mangaupdates::get_year(contents);
                auto img_url = mangaupdates::get_img_url(contents);

                // In the case no image url is found, get from either the archive or folder.jpeg
                if (img_url.empty() == true)
                    img_url = std::string("/mangapp/thumbnail/") + std::to_string(key);

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
                    { "year", year },
                    { "img-url", img_url }
                };

                on_event(std::move(context), true);

            }, on_error);
        }, on_error);
    }
}