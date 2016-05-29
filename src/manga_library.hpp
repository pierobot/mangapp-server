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

        typedef base::library<directory_type>::key_type key_type;

        manga_library(std::vector<std::wstring> const & library_paths, mangapp::users & usrs);
        manga_library(std::vector<std::string> const & library_paths, mangapp::users & usrs);
        manga_library(json11::Json const & library_paths, mangapp::users & usrs);
        virtual ~manga_library();
    protected:
        void search_title(manga_directory & manga,
                          std::function<void(std::string)> on_event,
                          unsigned int start_page = 1,
                          unsigned int max_pages = 5);
        virtual void search_online_source(manga_directory & manga, std::function<void(mstch::map&&, bool)> on_event) final;
    private:
        http_client m_http_client;

        void request_page(std::string const & name, unsigned int page_index, std::function<void(http_client::response_pointer &&)> on_event);
        void get_mangaupdates_cookie();
    };
}

#endif