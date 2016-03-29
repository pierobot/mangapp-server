#include "mangaupdates_parser.hpp"
#include "http_utility.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>

#include <utf8/utf8.h>

namespace
{
    static std::string const name_entry_search_str = "alt='Series Info'>";
    static std::string const id_search_str = "www.mangaupdates.com/series.html?id=";
    static std::string const desc_search_str = "<div class=\"sCat\"><b>Description</b></div>";
    static std::string const ass_names_search_str = "<div class=\"sCat\"><b>Associated Names</b></div>";
    static std::string const genres_search_str = "act=genresearch&amp;genre=";
    static std::string const authors_search_str = "<div class=\"sCat\"><b>Author(s)</b></div>";
    static std::string const artists_search_str = "<div class=\"sCat\"><b>Artist(s)</b></div>";
    static std::string const year_search_str = "<div class=\"sCat\"><b>Year</b></div>";
    static std::string const img_search_str = "<div class=\"sContent\" ><center><img";

    static std::pair<std::string, std::string> const junk_table[] =
    {
        { "<i>", "</i>" },  // Name junk in search results
        { "&nbsp; [", "]" } // Artists and Authors junk
    };

    enum junk_type
    {
        NAME_SEARCH = 0,
        ARTIST_AUTHOR,
    };

    static std::string & find_remove_junk(std::string & str, junk_type type)
    {
        auto const & start_pattern = junk_table[type].first;
        auto const & end_pattern = junk_table[type].second;

        auto start_pos = str.find(start_pattern);
        if (start_pos != std::string::npos)
        {
            auto end_pos = str.find(end_pattern, start_pos + start_pattern.length());
            if (end_pos != std::string::npos)
            {
                str.erase(start_pos, start_pattern.length());
                str.erase(end_pos - start_pattern.length(), end_pattern.length());
            }
        }

        return str;
    }

    //https://en.wikipedia.org/wiki/Levenshtein_distance
    static uint32_t levenshtein_distance(std::string const & s,
        std::string const & t)
    {
        if (s == t)
            return 0;
        if (s.length() == 0)
            return t.length();
        if (t.length() == 0)
            return s.length();

        std::vector<uint32_t> v0(t.length() + 1);
        std::vector<uint32_t> v1(t.length() + 1);

        auto n = 0;
        std::generate(v0.begin(), v0.end(), [&n]() { return n++; });

        for (size_t i = 0; i < s.length(); i++)
        {
            v1[0] = i + 1;

            for (size_t j = 0; j < t.length(); j++)
            {
                auto cost = s[i] == t[j] ? 0 : 1;
                v1[j + 1] = std::min({ v1[j] + 1,
                    v0[j + 1],
                    v0[j] + cost });
            }

            v0 = v1;
        }

        return v1[t.length()];
    }
}

std::string const mangaupdates::get_id(std::string const & contents, std::string const & name)
{
    using match_type = std::pair<float, std::string>;
    std::vector<match_type> matches;
    std::string id;

    // mangaupdates sends the names NCR encoded, so we must encode ours to NCR as well
    auto ncr_name = http_utility::encode_ncr(name);
    size_t id_start_pos = contents.find(id_search_str);
    // Iterate through every search result entry
    while (id_start_pos != std::string::npos)
    {
        id_start_pos += id_search_str.length();
        size_t id_end_pos = contents.find("'", id_start_pos);
        if (id_end_pos == std::string::npos)
            break;
        
        id = contents.substr(id_start_pos, id_end_pos - id_start_pos);
        size_t name_start_pos = contents.find(name_entry_search_str, id_start_pos);
        if (name_start_pos != std::string::npos)
        {
            name_start_pos += name_entry_search_str.length();
            size_t name_end_pos = contents.find("</a>", name_start_pos);
            if (name_end_pos == std::string::npos)
                break;
            // Get the string from positions and remove any junk
            auto potential_match = contents.substr(name_start_pos, name_end_pos - name_start_pos);
            potential_match = find_remove_junk(potential_match, NAME_SEARCH);
            // Do the names match?
            if (potential_match == ncr_name)
                return id;

            auto larger_length = std::max(potential_match.length(), name.length());
            auto match_percentage = 1.0f - (static_cast<float>(levenshtein_distance(potential_match, name)) / larger_length);

            matches.emplace_back(match_percentage, id);
        }

        id_start_pos = contents.find(id_search_str, id_start_pos);
    }

    // Sort the potential matches based on their match percentage; the frontmost being the highest
    std::sort(matches.begin(), matches.end(), 
        [](match_type const & left, match_type const & right) -> bool
    {
        return left.first > right.first;
    });

    return matches.size() > 0 ? matches.front().second : "";
}

std::string const mangaupdates::get_description(std::string const & contents)
{
    std::string description;

    size_t start_pos = contents.find(desc_search_str);
    if (start_pos != std::string::npos)
    {
        start_pos += desc_search_str.length();

        start_pos = contents.find(">", start_pos);
        if (start_pos != std::string::npos)
        {
            ++start_pos;

            size_t end_pos = contents.find("</div>", start_pos);
            if (end_pos != std::string::npos)
                description = contents.substr(start_pos, end_pos - start_pos);
        }
    }

    return description;
}

std::vector<std::string> const mangaupdates::get_associated_names(std::string const & contents)
{
    std::vector<std::string> associated_names;

    size_t start_pos = contents.find(ass_names_search_str);
    if (start_pos != std::string::npos)
    {
        start_pos += ass_names_search_str.length();
    
        size_t div_end_pos = contents.find("</div>", start_pos);
        if (div_end_pos != std::string::npos)
        {
            --div_end_pos;

            size_t end_pos = 0;
            start_pos = contents.find(">", start_pos);
            if (start_pos != std::string::npos)
            {
                ++start_pos;

                for (size_t i = 0; start_pos < div_end_pos; i++)
                {
                    end_pos = contents.find("<br />", start_pos);
                    if (end_pos == std::string::npos)
                        break;

                    associated_names.emplace_back(contents.substr(start_pos, end_pos - start_pos));
                    start_pos = end_pos + 6;
                }
            }
        }
    }
        
    return associated_names;
}

std::vector<std::string> const mangaupdates::get_genres(std::string const & contents)
{
    std::vector<std::string> genres;

    size_t start_pos = contents.find(genres_search_str);
    if (start_pos != std::string::npos)
    {
        start_pos += genres_search_str.length();

        size_t end_pos = 0;
        size_t div_end_pos = contents.find("</div>", start_pos);
        if (div_end_pos != std::string::npos)
        {
            --div_end_pos;

            for (size_t i = 0; start_pos < div_end_pos; i++)
            {
                start_pos = contents.find("<u>", start_pos);
                if (start_pos == std::string::npos)
                    break;

                start_pos += 3;
                end_pos = contents.find("</u>", start_pos);
                if (end_pos == std::string::npos)
                    break;

                genres.emplace_back(contents.substr(start_pos, end_pos - start_pos));
                start_pos = end_pos + 4;
            }

			/* ******** IMPROVE THIS ******  */
            // Remove the last two elements as they're rubbish we don't need
            genres.pop_back();
            genres.pop_back();
			/* ***************************** */
        }
    }

    return genres;
}

std::vector<std::string> const mangaupdates::get_authors(std::string const & contents)
{
    std::vector<std::string> authors;

    size_t start_pos = contents.find(authors_search_str);
    if (start_pos != std::string::npos)
    {
        start_pos += authors_search_str.length();

        size_t end_pos = 0;
        size_t div_end_pos = contents.find("</div>", start_pos);

        if (div_end_pos != std::string::npos)
        {
            --div_end_pos;

            for (size_t i = 0; start_pos < div_end_pos; i++)
            {
                start_pos = contents.find("<u>", start_pos);
                if (start_pos == std::string::npos)
                    break;

                start_pos += 3;
                end_pos = contents.find("</u></a><BR>", start_pos);
                if (end_pos == std::string::npos)
                    break;

                authors.emplace_back(contents.substr(start_pos, end_pos - start_pos));
                start_pos = end_pos + 12;
            }
        }
    }

    return authors;
}

std::vector<std::string> const mangaupdates::get_artists(std::string const & contents)
{
    std::vector<std::string> artists;

    size_t start_pos = contents.find(artists_search_str);
    if (start_pos != std::string::npos)
    {
        start_pos += artists_search_str.length();

        size_t end_pos = 0;
        size_t div_end_pos = contents.find("</div>", start_pos);
        if (div_end_pos != std::string::npos)
        {
            --div_end_pos;

            for (size_t i = 0; start_pos < div_end_pos; i++)
            {
                start_pos = contents.find("<u>", start_pos);
                if (start_pos == std::string::npos)
                    break;

                start_pos += 3;
                end_pos = contents.find("</u></a><BR>", start_pos);
                if (end_pos == std::string::npos)
                    break;

                artists.emplace_back(contents.substr(start_pos, end_pos - start_pos));
                start_pos = end_pos + 12;
            }
        }
    }

    return artists;
}

std::string const mangaupdates::get_year(std::string const & contents)
{
    std::string year;

    size_t start_pos = contents.find(year_search_str);
    if (start_pos != std::string::npos)
    {
        start_pos += year_search_str.length();
        start_pos = contents.find(">", start_pos);
        if (start_pos != std::string::npos)
        {
            ++start_pos;
            size_t end_pos = contents.find("</div>", start_pos);
            if (end_pos != std::string::npos)
            {
                --end_pos; // new line character
                year = contents.substr(start_pos, end_pos - start_pos);
            }
        }
    }

    return year;
}

std::string const mangaupdates::get_img_url(std::string const & contents)
{
    std::string img_url;

    size_t start_pos = contents.find(img_search_str);
    if (start_pos != std::string::npos)
    {
        start_pos += img_search_str.length();
        start_pos = contents.find("src='", start_pos);
        if (start_pos != std::string::npos)
        {
            start_pos += 5;
            size_t end_pos = contents.find('\'', start_pos);
            if (end_pos != std::string::npos)
            {
                img_url = contents.substr(start_pos, end_pos - start_pos);
            }
        }
    }

    return img_url;
}