#include "manga_library.hpp"
#include "http_utility.hpp"
#include "mangaupdates.hpp"
#include "utf8.hpp"

#include <fstream>
#include <iostream>
#include <mutex>
#include <numeric>

#include <json11/json11.hpp>

#include <boost/functional/hash.hpp>

#include <mstch/mstch.hpp>

extern std::mutex g_mutex_cout;

namespace mangapp
{
    std::unordered_map<std::string, std::string> g_mangaupdates_cookies;
    
    manga_library::manga_library(std::vector<std::wstring> const & library_paths, mangapp::users & users, std::string const & database_file) :
        library<manga_directory>(library_paths, users, database_file),
        m_http_client(http_version::v1_1)
    {
        if (g_mangaupdates_cookies.size() == 0)
            get_mangaupdates_cookie();
    }

    manga_library::manga_library(std::vector<std::string> const & library_paths, mangapp::users & users, std::string const & database_file) :
        library<manga_directory>(library_paths, users, database_file),
        m_http_client(http_version::v1_1)
    {
        if (g_mangaupdates_cookies.size() == 0)
            get_mangaupdates_cookie();
    }

    manga_library::manga_library(json11::Json const & settings_json, mangapp::users & users, std::string const & database_file) :
        library<manga_directory>(settings_json, users, database_file),
        m_http_client(http_version::v1_1)
    {
        if (g_mangaupdates_cookies.size() == 0)
            get_mangaupdates_cookie();

        auto on_error = [](std::string const & error_msg)
        {
            std::lock_guard<std::mutex> lock(g_mutex_cout);
            std::cout << error_msg << std::endl;
        };

        http_client::request_pointer request_ptr(new http_request(http_protocol::https, http_action::get, "www.mangaupdates.com", "/image/i189392.png"));
        if (request_ptr == nullptr)
        {
            on_error(std::string(__func__) + " - Unable to allocate http_client::request_pointer.");
            return;
        }

        //request_ptr->add_header("Accept", "text/html");
        request_ptr->add_header("Accept-Encoding", "gzip, deflate");
        request_ptr->add_header("Connection", "close");
        request_ptr->add_header("Host", "www.mangaupdates.com");
        request_ptr->add_header("DNT", "1");
        request_ptr->add_header("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36");

        request_ptr->add_header("Cookie", "secure_session=" + g_mangaupdates_cookies["secure_session"]);

        m_http_client.send(std::move(request_ptr),
            [on_error](http_client::response_pointer && response) -> void
        {
            if (response->get_code() == 200)
            {
                {
                    auto const & str = response->get_body();
                    std::ofstream f("b.jpeg", std::ios::binary);

                    f.write(str.c_str(), str.size());
                }
            }
        }, on_error);
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

        m_http_client.send(std::move(request_ptr),
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
                                     response_event on_event)
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

        m_http_client.send(std::move(request_title), on_event, on_error);
    }

    void manga_library::search_title(manga_directory & manga,
                                     string_event on_event,
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
                    if (start_page != num_pages && ++start_page <= max_pages)
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

    mstch::map manga_library::build_series_context(mangaupdates::series const & series) const
    {
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

        return mstch::map({
            { "names-list", names_array },
            { "description", description },
            { "authors-list", authors_array },
            { "artists-list", artists_array },
            { "genres-list", genres_array },
            { "year", year },
            { "img-url", img_url } });
    }

    void manga_library::on_id(std::string const & id, manga_directory & manga, context_event on_event, string_event on_error)
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
                return;
            }

            auto const & contents = response->get_body();
            mangaupdates::series series(contents, to_utf8(manga.get_name()), manga.get_key(), id);
            save_details(manga.get_key(), series);

            auto context = build_series_context(series);
            /*manga.set_series(std::move(series));*/

            on_event(std::move(context), true);
        }, on_error);
    }

    void manga_library::search_online_source(manga_directory & manga, context_event on_event)
    {
        auto on_error = [on_event](std::string const & error_msg)
        {
            std::lock_guard<std::mutex> lock(g_mutex_cout);
            std::cout << error_msg << std::endl;

            on_event(mstch::map({}), false);
        };

        // Find the manga in our library
        auto manga_iterator = library::find(manga.get_key());
        if (manga_iterator != library::end())
        {
            // Do we have the series' details in our database?
            mangaupdates::series series = query_details(manga.get_key());
            if (series.get_id().empty() == false)
            {
                auto context = build_series_context(series);
                on_event(std::move(context), true);
            }
            else
            {
                // No, we must search for the series on mangaupdates
                search_title(manga_iterator->second,
                    [this, &manga, on_event, on_error](std::string const & id) -> void
                {
                    on_id(id, manga, on_event, on_error);
                });
            }
        }
    }

    void manga_library::save_cover(size_t key, std::string const & id, std::string const & image)
    {
        static std::string save_command = "INSERT INTO images (key, id, image) VALUES (@key, @id, @image)";
        library::sqlite3_execute(save_command, { { "key", std::to_string(key) }, { "@id", id }, { "@image", image } });
    }

    void manga_library::save_details(key_type key, mangaupdates::series const & series)
    {
        static std::string save_command = "INSERT INTO details (key, id, artists, authors, description, genres, names, year) "
                                          "VALUES (@key, @id, @artists, @authors, @description, @genres, @names, @year)";

        json11::Json artists(series.get_artists());
        json11::Json authors(series.get_authors());
        json11::Json genres(series.get_genres());
        json11::Json names(series.get_associated_names());

        library::sqlite3_execute(save_command,
        {
            { "@key", std::to_string(key) },
            { "@id", series.get_id() },
            { "@artists", artists.dump() },
            { "@authors", authors.dump() },
            { "@description", series.get_description() },
            { "@genres", genres.dump() },
            { "@names", names.dump() },
            { "@year", series.get_year() }
        });
    }

    auto manga_library::query_details(key_type key) const -> mangaupdates::series
    {
        static std::string query_str = "SELECT * FROM details WHERE key = @key";
        
        mangaupdates::series series;
        
        library::sqlite3_query(query_str.c_str(), { { "@key", std::to_string(key) } },
            [key, &series](sqlite3pp::query::iterator begin, sqlite3pp::query::iterator end)
        {
            if (begin != end)
            {
                std::string id;
                std::string artists_str;
                std::string authors_str;
                std::string description;
                std::string genres_str;
                std::string names_str;
                std::string year;

                (*begin).getter() >> sqlite3pp::ignore >> id >> artists_str >> authors_str >> description >> genres_str >> names_str >> year;
               
                series = std::move(mangaupdates::series(key, std::move(id), std::move(description), 
                                                        std::move(names_str), std::string(), std::move(genres_str),
                                                        std::move(authors_str), std::move(artists_str), std::move(year)));
            }
            
        });

        return series;
    }
}