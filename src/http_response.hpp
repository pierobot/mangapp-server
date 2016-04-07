#ifndef MANGAPP_HTTP_RESPONSE_HPP
#define MANGAPP_HTTP_RESPONSE_HPP

#include <string>
#include <unordered_map>
#include <vector>

namespace mangapp
{
    class http_response
    {
    public:
        typedef std::unordered_multimap<std::string, std::string> headers_map_type;
        typedef std::vector<std::unordered_map<std::string, std::string>> cookies_vector_type;

        http_response(std::string && contents);
        ~http_response();

        int32_t const get_code() const;
        headers_map_type const & get_headers() const;
        std::string const & get_header_value(std::string const & name) const;
        cookies_vector_type const & get_cookies() const;
        std::string const & get_body() const;
        std::string const & get_status() const;

        void set_body(std::string && body);
    protected:
    private:
        size_t m_offset;
        int32_t const m_code;
        std::string const m_status;
        headers_map_type const m_headers;
        cookies_vector_type const m_cookies;
        std::string m_body;

        int32_t const get_code_from(std::string const & contents, size_t & offset) const;
        std::string const get_status_from(std::string const & contents, size_t & offset) const;
        headers_map_type const get_headers_from(std::string const & contents, size_t & offset) const;
        cookies_vector_type const get_cookies_from(headers_map_type const & headers) const;
    };
}

#endif