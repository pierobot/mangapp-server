#ifndef UTF8_HPP
#define UTF8_HPP

#include <string>

#include <utf8/utf8.h>

std::string to_utf8(std::wstring const & str);
std::string to_utf8(std::wstring::const_iterator cbegin, std::wstring::const_iterator cend);

std::wstring to_utf16(std::string const & str);
std::wstring to_utf16(std::string::const_iterator cbegin, std::string::const_iterator cend);

#endif
