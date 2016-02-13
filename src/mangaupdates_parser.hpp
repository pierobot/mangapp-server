#ifndef MANGAPP_MANGAUPDATES_PARSER_HPP
#define MANGAPP_MANGAUPDATES_PARSER_HPP

#include <string>
#include <vector>

namespace mangaupdates
{
    std::string const encode_ncr(std::string const & str);
    std::string const get_id(std::string const & contents, std::string const & name);
    std::string const get_description(std::string const & contents);
    std::vector<std::string> const get_associated_names(std::string const & contents);
    std::vector<std::string> const get_genres(std::string const & contents);
    std::vector<std::string> const get_authors(std::string const & contents);
    std::vector<std::string> const get_artists(std::string const & contents);
    std::string const get_year(std::string const & contents);
};

#endif