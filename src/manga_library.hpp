#ifndef MANGAPP_MANGA_LIBRARY_HPP
#define MANGAPP_MANGA_LIBRARY_HPP

#include "manga_directory_entry.hpp"
#include "library.hpp"
#include "manga_entry.hpp"
#include "http_helper.hpp"

#include <string>
#include <vector>

namespace json11
{
    class Json;
}

static std::vector<std::wstring> const g_archive_extensions = { L".rar", L".cbr",
                                                                L".zip", L".cbz",
                                                                L".7z",  L".cb7" };

template<class Container, class Element>
static bool is_in_container(Container const & container, Element const & element)
{
    return std::find(container.cbegin(), container.cend(), element) != container.cend();
}

namespace mangapp
{
    class manga_library : virtual public base::library<manga_directory>
    {
    public:
        typedef manga_directory directory_type;
        typedef manga_entry file_entry_type;

        typedef base::library<directory_type>::key_type key_type;

        manga_library(std::vector<std::wstring> const & library_paths);
        manga_library(std::vector<std::string> const & library_paths);
        manga_library(json11::Json const & library_paths);
        virtual ~manga_library();
    protected:
        virtual void search_online_source(std::string const & name, std::function<void(mstch::map&&, bool)> on_event) final;
    private:
        http_helper m_http_helper;
    };
}

#endif