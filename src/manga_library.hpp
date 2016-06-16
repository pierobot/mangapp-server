#ifndef MANGAPP_MANGA_LIBRARY_HPP
#define MANGAPP_MANGA_LIBRARY_HPP

#include "manga_directory_entry.hpp"
#include "library.hpp"
#include "manga_entry.hpp"
#include "http_client.hpp"

namespace json11
{
    class Json;
}

namespace mangapp
{
    class manga_library : public base::library<manga_directory>
    {
    public:
        typedef manga_directory directory_type;
        typedef manga_entry file_entry_type;

        typedef std::function<void(mstch::map&&, bool)> context_event;
        typedef std::function<void(std::string const&)> string_event;
        typedef std::function<void(http_client::response_pointer &&)> response_event;

        typedef base::library<directory_type>::key_type key_type;

        manga_library(std::vector<std::wstring> const & library_paths, mangapp::users & users, std::string const & database_file);
        manga_library(std::vector<std::string> const & library_paths, mangapp::users & users, std::string const & database_file);
        manga_library(json11::Json const & settings_json, mangapp::users & users, std::string const & database_file);
        virtual ~manga_library();
    protected:
        virtual void search_online_source(manga_directory & manga, context_event on_event) final;
    private:
        http_client m_http_client;

        mstch::map build_series_context(mangaupdates::series const & series) const;

        void on_id(std::string const & id, manga_directory & manga, context_event on_event, string_event on_error);
        void search_title(manga_directory & manga,
                          string_event on_event,
                          unsigned int start_page = 1,
                          unsigned int max_pages = 5);
        void request_page(std::string const & name, unsigned int page_index, response_event on_event);

        void save_cover(key_type key, std::string const & id, std::string const & image);
        void save_details(key_type key, mangaupdates::series const & series);

        auto query_details(key_type key) const -> mangaupdates::series;
    };
}

#endif