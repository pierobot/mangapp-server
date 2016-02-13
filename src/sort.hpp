#ifndef SORT_HPP
#define SORT_HPP

#ifdef USE_BOOST_REGEX
#   include <boost/regex.hpp>
#else
#   include <regex>
#endif
#include <string>

namespace sort
{
    namespace detail
    {
        template<class CharType>
        struct numerical_regex_base
        {
            typedef CharType char_type;
            typedef std::basic_string<char_type> string_type;
#ifdef USE_BOOST_REGEX
            typedef boost::basic_regex<char_type> regex_type;
            typedef boost::regex_token_iterator<typename string_type::const_iterator> sregex_token_iterator;
#else
            typedef std::basic_regex<char_type> regex_type;
            typedef std::regex_token_iterator<typename string_type::const_iterator> sregex_token_iterator;
#endif
            int compare(string_type const & left, string_type const & right, string_type const & regex_str) const
            {
                regex_type regex(regex_str);
                sregex_token_iterator left_iterator(left.cbegin(), left.cend(), regex);
                sregex_token_iterator right_iterator(right.cbegin(), right.cend(), regex);
                sregex_token_iterator end;

                while (left_iterator != end && right_iterator != end)
                {
                    // Are the iterator token values equal?
                    if (*left_iterator != *right_iterator)
                        // No, get their numerical value and return their difference
                        return std::stol(left_iterator->str()) - std::stol(right_iterator->str());

                    // Yes, get the next tokens
                    ++left_iterator;
                    ++right_iterator;
                }

                if (left_iterator == end && right_iterator != end)
                    // left has fewer number tokens than right
                    return -1;
                else if (left_iterator != end && right_iterator == end)
                    // left has more number tokens than right
                    return 1;

                // Equal
                return 0;
            }
        };
    }

    struct numerical_regex_less : detail::numerical_regex_base<char>
    {
        bool operator()(std::string const & left, std::string const & right) const
        {
            return numerical_regex_base::compare(left, right, "\\d+") < 0;
        }
    };

    struct numerical_wregex_less : detail::numerical_regex_base<wchar_t>
    {
        bool operator()(std::wstring const & left, std::wstring const & right) const
        {
            return numerical_regex_base::compare(left, right, L"\\d+") < 0;
        }
    };

    struct numerical_regex_greater : detail::numerical_regex_base<char>
    {
        bool operator()(std::string const & left, std::string const & right) const
        {
            return numerical_regex_base::compare(left, right, "\\d+") > 0;
        }
    };

    struct numerical_wregex_greater : detail::numerical_regex_base<wchar_t>
    {
        bool operator()(std::wstring const & left, std::wstring const & right) const
        {
            return numerical_regex_base::compare(left, right, L"\\d+") > 0;
        }
    };
}

#endif