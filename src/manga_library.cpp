#include "manga_library.hpp"
#include "http_utility.hpp"
#include "mangaupdates_parser.hpp"
#include "utf8.hpp"
#include "http_request.hpp"

#include <fstream>
#include <iostream>
#include <mutex>

#include <json11/json11.hpp>

#include <boost/functional/hash.hpp>

#include <mstch/mstch.hpp>

extern std::mutex g_mutex_cout;

namespace mangapp
{
    manga_library::manga_library(std::vector<std::wstring> const & library_paths, mangapp::users & usrs) :
        library<manga_directory>(library_paths, usrs),
        m_http_client(http_version::v1_1)
    {
    }

    manga_library::manga_library(std::vector<std::string> const & library_paths, mangapp::users & usrs) :
        library<manga_directory>(library_paths, usrs),
        m_http_client(http_version::v1_1)
    {
    }

    manga_library::manga_library(json11::Json const & library_paths, mangapp::users & usrs) :
        library<manga_directory>(library_paths, usrs),
        m_http_client(http_version::v1_1)
    {
    }

    manga_library::~manga_library()
    {
    }

    void manga_library::search_online_source(key_type key,
                                             std::string const & name,
                                             std::function<void(mstch::map&&, bool)> on_event)
    {
        auto on_error = [on_event](std::string const & error_msg)
        {
            std::lock_guard<std::mutex> lock(g_mutex_cout);

            std::cout << error_msg << std::endl;

            on_event(mstch::map({}), false);
        };

        http_client::request_pointer request_title(new http_request(http_protocol::https, http_action::get, "www.mangaupdates.com", "/series.html"));
        if (request_title == nullptr)
        {
            on_error("Unable to allocate http_client::request_pointer");
        }

        request_title->add_parameter("page", "1");
        request_title->add_parameter("stype", "title");
        request_title->add_parameter("search", http_utility::encode_uri(http_utility::encode_ncr(name)));

        request_title->add_header("Accept", "text/html");
        request_title->add_header("Accept-Encoding", "gzip, deflate");
        request_title->add_header("Connection", "close");
        request_title->add_header("Host", "www.mangaupdates.com");
        request_title->add_header("DNT", "1");
        request_title->add_header("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36");
        
        m_http_client.send(request_title,
            [this, key, name, on_error, on_event](http_client::response_pointer && response)
        {
            if (response->get_code() != 200)
            {
                std::string message = std::string("Request to ") + response->get_header_value("Host") + " " + "failed: " + response->get_status();
                on_error(message);
            }

            auto series_id = mangaupdates::get_id(response->get_body(), name);
            if (series_id.empty() == true)
            {
                on_error(std::string("Unable to find manga with name: ") + name);
                return;
            }

            http_client::request_pointer request_id(new http_request(http_protocol::https, http_action::get, "www.mangaupdates.com", "/series.html"));
            if (request_id == nullptr)
            {
                on_error("Unable to allocate http_client::request_pointer");
            }

            request_id->add_parameter("id", series_id);

            request_id->add_header("Accept", "text/html");
            request_id->add_header("Accept-Encoding", "gzip, deflate");
            request_id->add_header("Connection", "close");
            request_id->add_header("Host", "www.mangaupdates.com");
            request_id->add_header("DNT", "1");
            request_id->add_header("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36");

            m_http_client.send(std::move(request_id),
                [this, key, series_id, on_error, on_event](http_client::response_pointer && response)
            {
                if (response->get_code() != 200)
                {
                    std::string message = std::string("Request to ") + response->get_header_value("Host") + " " + "failed: " + response->get_status();
                    on_error(message);
                }

                auto const & contents = response->get_body();
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