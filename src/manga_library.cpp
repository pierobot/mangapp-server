#include "manga_library.hpp"
#include "http_utility.hpp"
#include "mangaupdates.hpp"
#include "utf8.hpp"

#include <fstream>
#include <iostream>
#include <mutex>

#include <json11/json11.hpp>

#include <boost/functional/hash.hpp>

#include <mstch/mstch.hpp>

extern std::mutex g_mutex_cout;

namespace mangapp
{
    std::unordered_map<std::string, std::string> g_mangaupdates_cookies;
    
    manga_library::manga_library(std::vector<std::wstring> const & library_paths, mangapp::users & usrs) :
        library<manga_directory>(library_paths, usrs),
        m_http_client(http_version::v1_1)
    {
        if (g_mangaupdates_cookies.size() == 0)
            get_mangaupdates_cookie();
    }

    manga_library::manga_library(std::vector<std::string> const & library_paths, mangapp::users & usrs) :
        library<manga_directory>(library_paths, usrs),
        m_http_client(http_version::v1_1)
    {
        if (g_mangaupdates_cookies.size() == 0)
            get_mangaupdates_cookie();
    }

    manga_library::manga_library(json11::Json const & library_paths, mangapp::users & usrs) :
        library<manga_directory>(library_paths, usrs),
        m_http_client(http_version::v1_1)
    {
        if (g_mangaupdates_cookies.size() == 0)
            get_mangaupdates_cookie();
    }

    manga_library::~manga_library()
    {
    }  

    void manga_library::get_mangaupdates_cookie()
    {
        auto on_error = [](std::string const & error_msg)
        {
            std::lock_guard<std::mutex> lock(g_mutex_cout);
            std::cout << error_msg << std::endl;
        };

        http_client::request_pointer request_ptr(new http_request(http_protocol::https, http_action::get, "www.mangaupdates.com", "/"));
        if (request_ptr == nullptr)
        {
            on_error(std::string(__func__) + " - Unable to allocate http_client::request_pointer.");
            return;
        }

        request_ptr->add_header("Accept", "text/html");
        request_ptr->add_header("Accept-Encoding", "gzip, deflate");
        request_ptr->add_header("Connection", "close");
        request_ptr->add_header("Host", "www.mangaupdates.com");
        request_ptr->add_header("DNT", "1");
        request_ptr->add_header("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36");

        m_http_client.send(request_ptr,
            [on_error](http_client::response_pointer && response) -> void
        {
            if (response->get_code() == 200)
            {
                for (auto const & cookies : response->get_cookies())
                {
                    auto const session_iterator = cookies.find("secure_session");
                    if (session_iterator != cookies.cend())
                    {
                        g_mangaupdates_cookies.emplace(session_iterator->first, session_iterator->second);
                    }
                }

                if (g_mangaupdates_cookies.size() == 0)
                    on_error(std::string(__func__) + " - Unable to find secure_session cookie.");
            }
            else
            {
                std::string message = std::string(__func__) + " - Request to " + response->get_header_value("Host") + " " + "failed: " + response->get_status();
                on_error(message);
            }

        }, on_error);
    }

    void manga_library::request_page(std::string const & name,
                                     unsigned int page_index,
                                     std::function<void(http_client::response_pointer &&)> on_event)
    {
        auto on_error = [on_event](std::string const & error_msg)
        {
            std::lock_guard<std::mutex> lock(g_mutex_cout);
            std::cout << error_msg << std::endl;

            on_event(nullptr);
        };

        http_client::request_pointer request_title(new http_request(http_protocol::https, http_action::get, "www.mangaupdates.com", "/series.html"));
        if (request_title == nullptr)
        {
            on_error("manga_library::search_title::request_page_lambda - Unable to allocate http_client::request_pointer.");
            return;
        }

        request_title->add_parameter("page", std::to_string(page_index));
        request_title->add_parameter("stype", "title");
        request_title->add_parameter("search", http_utility::encode_uri(http_utility::encode_ncr(name)));

        request_title->add_header("Accept", "text/html");
        request_title->add_header("Accept-Encoding", "gzip, deflate");
        request_title->add_header("Connection", "close");
        request_title->add_header("Host", "www.mangaupdates.com");
        request_title->add_header("DNT", "1");
        request_title->add_header("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36");

        m_http_client.send(request_title, on_event, on_error);
    }

    void manga_library::search_title(manga_directory & manga,
                                     std::function<void(std::string)> on_event,
                                     unsigned int start_page,
                                     unsigned int max_pages)
    {
        auto const name = to_utf8(manga.get_name());
        request_page(name, start_page,
            [this, &manga, name, on_event, start_page, max_pages](http_client::response_pointer && response_ptr) mutable -> void
        {
            std::string const & contents = response_ptr->get_body();
            unsigned int num_pages = mangaupdates::get_num_pages(contents);           
            auto page_matches = mangaupdates::get_page_matches(contents, name);
            if (page_matches.size() > 0)
            {
                // Is there a perfect match?
                if (page_matches.front().first == 1.0f)
                {
                    // Yes, we have the id
                    on_event(page_matches.front().second);
                }
                else
                {
                    // No, keep the results and try again on the next page
                    manga.get_series().add_possible_matches(std::move(page_matches));
                    if (++start_page <= max_pages)
                        search_title(manga, on_event, start_page, max_pages);
                    else
                        on_event(manga.get_series().get_best_match().second);
                }
            }
            else
            {
                // We found nothing
                on_event("");
            }
        });
    }

    void manga_library::search_online_source(manga_directory & manga, std::function<void(mstch::map&&, bool)> on_event)
    {
        auto on_error = [on_event](std::string const & error_msg)
        {
            std::lock_guard<std::mutex> lock(g_mutex_cout);
            std::cout << error_msg << std::endl;

            on_event(mstch::map({}), false);
        };

        auto on_id = [this, &manga, on_event, on_error](std::string id) -> void
        {
            if (id.empty() == true)
            {
                on_error(std::string(__func__) + " - Unable to find id for " + to_utf8(manga.get_name()));
                return;
            }

            http_client::request_pointer request_id(new http_request(http_protocol::https, http_action::get, "www.mangaupdates.com", "/series.html"));
            if (request_id == nullptr)
            {
                on_error(std::string(__func__) + " - Unable to allocate http_client::request_pointer.");
                return;
            }

            request_id->add_parameter("id", id);
            request_id->add_header("Accept", "text/html");
            request_id->add_header("Accept-Encoding", "gzip, deflate");
            request_id->add_header("Connection", "close");
            request_id->add_header("Host", "www.mangaupdates.com");
            request_id->add_header("DNT", "1");
            request_id->add_header("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36");

            m_http_client.send(std::move(request_id),
                [this, &manga, id, on_event, on_error](http_client::response_pointer && response)
            {
                if (response->get_code() != 200)
                {
                    std::string message = std::string("Request to ") + response->get_header_value("Host") + " " + "failed: " + response->get_status();
                    on_error(message);
                }

                auto const & contents = response->get_body();
                mangaupdates::series series(contents, to_utf8(manga.get_name()), manga.get_key(), id);
                auto const & associated_names = series.get_associated_names();
                auto const & description = series.get_description();
                auto const & genres = series.get_genres();
                auto const & authors = series.get_authors();
                auto const & artists = series.get_artists();
                auto const & year = series.get_year();
                auto const & img_url = series.get_img_url();

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
                    { "authors-list", authors_array },
                    { "artists-list", artists_array },
                    { "genres-list", genres_array },
                    { "year", year },
                    { "img-url", img_url }
                };

                on_event(std::move(context), true);
            }, on_error);
        };

        auto manga_iterator = library::find(manga.get_key());
        if (manga_iterator != library::end())
        {
            search_title(manga_iterator->second, on_id);
        }
    }
}