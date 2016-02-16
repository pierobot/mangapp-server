#ifndef MANGAPP_MANGA_LIBRARY_HPP
#define MANGAPP_MANGA_LIBRARY_HPP

#include "manga_directory_entry.hpp"
#include "library.hpp"
#include "manga_entry.hpp"
#include "http_helper.hpp"

#include <string>
#include <vector>

#include <json11/json11.hpp>

static std::vector<std::wstring> const g_archive_extensions = { L".rar", L".cbr",
                                                                L".zip", L".cbz",
                                                                L".7z",  L".cb7" };

template<class Container, class Element>
static inline bool is_in_container(Container const & container, Element const & element)
{
    return std::find(container.cbegin(), container.cend(), element) != container.cend();
}

namespace mangapp
{
    class manga_library : public base::library<manga_directory>
    {
    public:
        typedef base::library<manga_directory>::key_type key_type;

        manga_library(std::vector<std::wstring> const & library_paths);
        manga_library(std::vector<std::string> const & library_paths);
        manga_library(json11::Json const & library_paths);
        virtual ~manga_library();

        /**
        *   Converts the manga entries to a JSON object.
        *
        *   @return a JSON object representing the manga entries
        */
        json11::Json const get_list() const;

        json11::Json const get_files(size_t key) const;

        /**
        *   Reads the thumbnail of a manga from its folder or from an archive.
        *   Retrieval from an archive is NOT implemented yet.
        *
        *   @param key the key of the manga
        *   @param folder indicates whether or not to retrieve from the folder or archive
        *   @param index index of the archive in the folder
        *   @return a string containing the thumbnail's contents
        */
        std::string get_thumbnail(key_type key, bool folder, uint32_t index = 0) const;

        /**
        *   Gets the details of a manga from mangaupdates.com
        *
        *   @param key the key of the manga
        *   @param on_event a function object that will be called on success or failure
        */
        void get_manga_details(key_type key, std::function<void(json11::Json)> on_event);

        /** 
        *   Gets the desired image from an archive.
        *
        *   @param key the key of the manga
        *   @param key the key of the archive file
        *   @param index the index of the image
        *   @return a string containing the image's contents
        */
        std::string get_image(key_type key, key_type file_key, size_t index) const;
    protected:
        /**
        *   Asynchronously searches mangaupdates for the supplied name of the manga.
        *   The results are stored in a JSON object and passed to the 'on_event' callback.
        *
        *   @param name the name of the manga
        *   @param on_event a callback that handles the JSON object
        */
        void search_mangaupdates_for(std::string const & name, std::function<void(json11::Json)> on_event);
    private:
        http_helper m_http_helper;
    };
}

#endif