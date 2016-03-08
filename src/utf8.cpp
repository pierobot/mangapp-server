#include "utf8.hpp"

#include <iterator>

std::string to_utf8(std::wstring const & str)
{
    std::string utf8_str;
    utf8::utf16to8(str.begin(), str.end(), std::back_inserter(utf8_str));

    return utf8_str;
}

std::string to_utf8(std::wstring::const_iterator cbegin, std::wstring::const_iterator cend)
{
    std::string utf8_str;
    utf8::utf16to8(cbegin, cend, std::back_inserter(utf8_str));

    return utf8_str;
}

std::wstring to_utf16(std::string const & str)
{
    std::wstring utf16_str;
    utf8::utf8to16(str.cbegin(), str.cend(), std::back_inserter(utf16_str));

    return utf16_str;
}

std::wstring to_utf16(std::string::const_iterator cbegin, std::string::const_iterator cend)
{
    std::wstring utf16_str;
    utf8::utf8to16(cbegin, cend, std::back_inserter(utf16_str));

    return utf16_str;
}
