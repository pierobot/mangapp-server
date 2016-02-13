#ifndef MANGAPP_BASE64_HPP
#define MANGAPP_BASE64_HPP

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

namespace mangapp
{
    // Shamelessly stolen from http://stackoverflow.com/a/28471421
    inline std::string encode64(const std::string &val)
    {
        using namespace boost::archive::iterators;
        using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
        auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
        return tmp.append((3 - val.size() % 3) % 3, '=');
    }
}

#endif