#include "http_response.hpp"

#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>

namespace mangapp
{
    http_response::http_response(std::string && contents) :
        m_offset(0),
        m_code(get_code_from(contents, m_offset)),
        m_status(get_status_from(contents, m_offset)),
        m_headers(get_headers_from(contents, m_offset)),
        m_cookies(get_cookies_from(m_headers))
    {
    }

    http_response::~http_response()
    {
    }

    int32_t const http_response::get_code() const
    {
        return m_code;
    }

    auto http_response::get_headers() const -> headers_map_type const &
    {
        return m_headers;
    }

    auto http_response::get_cookies() const -> cookies_vector_type const &
    {
        return m_cookies;
    }

    std::string const & http_response::get_header_value(std::string const & name) const
    {
        static std::string const empty_value;

        auto iterator = m_headers.find(name);
        if (iterator != m_headers.cend())
        {
            return iterator->second;
        }

        return empty_value;
    }

    std::string const & http_response::get_body() const
    {
        return m_body;
    }

    std::string const & http_response::get_status() const
    {
        return m_status;
    }

    int32_t const http_response::get_code_from(std::string const & contents, size_t & offset) const
    {
        int32_t code = -1;

        size_t start_pos = contents.find(' ');
        if (start_pos != std::string::npos)
        {
            ++start_pos;
            size_t end_pos = contents.find(' ', start_pos);
            if (end_pos != std::string::npos)
            {
                code = std::stol(contents.substr(start_pos, end_pos - start_pos));
                offset = end_pos + 1;
            }
        }

        return code;
    }

    std::string const http_response::get_status_from(std::string const & contents, size_t & offset) const
    {
        std::string status;

        size_t start_pos = offset;
        size_t end_pos = contents.find("\r\n");
        if (end_pos != std::string::npos)
        {
            status = contents.substr(start_pos, end_pos - start_pos);
            offset = end_pos + 2;
        }

        return status;
    }

    auto http_response::get_headers_from(std::string const & contents, size_t & offset) const -> headers_map_type const
    {
        headers_map_type headers;

        size_t line_start_pos = offset;
        size_t line_end_pos = contents.find("\r\n", line_start_pos);
        while (line_end_pos != std::string::npos)
        {
            size_t key_start_pos = line_start_pos;
            size_t key_end_pos = contents.find(": ", line_start_pos);
            if (key_end_pos == std::string::npos)
                break;

            size_t value_start_pos = key_end_pos + 2;
            size_t value_end_pos = contents.find("\r\n", key_end_pos);

            std::string key = contents.substr(key_start_pos, key_end_pos - key_start_pos);
            std::string value = contents.substr(value_start_pos, value_end_pos - value_start_pos);

            headers.emplace(std::move(key), std::move(value));

            line_start_pos = value_end_pos + 2;

            line_end_pos = contents.find("\r\n", line_start_pos);
        }

        return headers;
    }

    auto http_response::get_cookies_from(headers_map_type const & headers) const -> cookies_vector_type const
    {
        cookies_vector_type cookies;

        auto cookie_range = headers.equal_range("Set-Cookie");
        for (auto cookie_iterator = cookie_range.first; cookie_iterator != cookie_range.second; ++cookie_iterator)
        {
            std::unordered_map<std::string, std::string> map;
            auto const & cookie_value = cookie_iterator->second;
            auto const tokens = boost::tokenizer<boost::escaped_list_separator<char>>(cookie_value, boost::escaped_list_separator<char>{'\\', ';', '"'});
            
            for (auto const & token : tokens)
            {
                size_t key_start_pos = token.find('=');
                if (key_start_pos == std::string::npos)
                    break;

                auto key = token.substr(0, key_start_pos++);
                auto value = token.substr(key_start_pos);

                boost::trim(key);
                boost::trim(value);

                map.emplace(std::move(key), std::move(value));
            }
            
            if (map.size() > 0)
                cookies.emplace_back(std::move(map));
        }
       
        return cookies;
    }

    void http_response::set_body(std::string && body)
    {
        m_body = body;
    }
}