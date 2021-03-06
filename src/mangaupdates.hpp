#ifndef MANGAPP_MANGAUPDATES_HPP
#define MANGAPP_MANGAUPDATES_HPP

#include <string>
#include <vector>

#include <Jaro-Winkler/jaroWinkler.hpp>

#include <json11/json11.hpp>

namespace mangaupdates
{
    unsigned int const get_num_pages(std::string const & contents);
    auto get_page_matches(std::string const & contents, std::string const & name) -> std::vector<std::pair<float, std::string>>;

    class series
    {
    public:


        series(size_t key);
        series(std::string const & contents, std::string const & name, size_t key, std::string const & id = "");
        series(size_t key,
               std::string && id,
               std::string && description,
               std::string && assoc_names,
               std::string && img_url,
               std::string && genres,
               std::string && authors,
               std::string && artists,
               std::string && year);
        series(series const & s);
        series(series && s);

        series & operator=(series const &) = delete;
        series & operator=(series && s);

        virtual ~series();

        size_t const get_key() const { return m_key; }
        std::string const & get_id() const { return m_id; }
        std::string const & get_description() const { return m_description; }
        std::vector<std::string> const & get_associated_names() const { return m_assoc_names; }
        std::vector<std::string> const & get_genres() const { return m_genres; }
        std::vector<std::string> const & get_authors() const { return m_authors; }
        std::vector<std::string> const & get_artists() const { return m_artists; }
        std::string const & get_year() const { return m_year; }
        std::string const & get_img_url() const { return m_img_url; }
    protected:
    private:
        size_t m_key;
        size_t m_current_pos;
        float m_levenshtein_distance;
        std::string m_id;
        std::string m_description;
        std::vector<std::string> m_assoc_names;
        std::string m_img_url;
        std::vector<std::string> m_genres;
        std::vector<std::string> m_authors;
        std::vector<std::string> m_artists;
        std::string m_year;
    };
};

#endif