#ifndef MANGAPP_NATSORT_HPP
#define MANGAPP_NATSORT_HPP

#include <string>

#include <utf8/utf8.h>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

namespace natural
{
    namespace detail
    {
        template<class CharType>
        inline unsigned long strtoul(CharType const * str, CharType ** end_ptr = nullptr, int base = 10)
        {
            return 0;
        }

        template<>
        inline unsigned long strtoul<char>(char const * str, char ** end_ptr, int base)
        {
            return ::strtoul(str, end_ptr, base);
        }

        template<>
        inline unsigned long strtoul<wchar_t>(wchar_t const * str, wchar_t ** end_ptr, int base)
        {
            return ::wcstoul(str, end_ptr, base);
        }
    }

    template<class CharType>
    int compare(std::basic_string<CharType> const & left, std::basic_string<CharType> const & right)
    {      
        auto left_ptr = left.c_str();
        auto right_ptr = right.c_str();
          
        // Iterate until we reach the end of one of the strings
        while (*left_ptr && *right_ptr)
        {
            // Are they both digits?
            if (boost::algorithm::is_digit()(*left_ptr) && boost::algorithm::is_digit()(*right_ptr))
            {
                // Yes, get their values
                char * left_end_ptr = nullptr;
                char * right_end_ptr = nullptr;

                auto left_num = detail::strtoul(left_ptr, &left_end_ptr, 10);
                auto right_num = detail::strtoul(right_ptr, &right_end_ptr, 10);
                
                // Are they equal?
                if (left_num != right_num)
                {
                    // No, return the difference. 
                    return left_num - right_num;
                }
                else
                {
                    // Yes, continue from the next character after the digits
                    left_ptr = left_end_ptr;
                    right_ptr = right_end_ptr;
                    continue;
                }
            }
            // Are they both alphabetical?
            else if (boost::algorithm::is_alpha()(*left_ptr) && boost::algorithm::is_alpha()(*right_ptr))
            {
                // Yes, now are they the same?
                if (*left_ptr != *right_ptr)
                    // No, return the difference
                    return *left_ptr - *right_ptr;

                // They're equal so leave this scope and advance left_ptr and right_ptr 
            }
            else
            {
                // There's one alphabetical character and a number. The number always comes first. 
                return boost::algorithm::is_digit()(*left_ptr) ? -1 : 1;
            }

            // Advance our pointers
            ++left_ptr;
            ++right_ptr;
        }

        return 0;
    }

    template<class CharType>
    struct sort_less
    {
        bool operator()(std::basic_string<CharType> const & left, std::basic_string<CharType> const & right)
        {
            return compare(left, right) < 0;
        }
    };
    
    template<class CharType>
    struct sort_greater
    {
        bool operator()(std::basic_string<CharType> const & left, std::basic_string<CharType> const & right)
        {
            return compare(left, right) > 0;
        }
    };
}

#endif