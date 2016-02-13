#ifndef MANGAPP_HTTP_UTILITY_HPP
#define MANGAPP_HTTP_UTILITY_HPP

#include <string>

namespace http_utility
{
    std::string const encode_ncr(std::string const & str);
    std::string const encode_uri(std::string const & str);
}

#endif