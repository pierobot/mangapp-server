#include "http_request.hpp"

namespace mangapp
{
    http_request::http_request(http_protocol protocol, std::string host, std::string url, std::string body, parameters_init_type params/*= {}*/) :
        m_protocol(protocol),
        m_host(std::move(host)),
        m_url(std::move(url)),
        m_body(std::move(body)),
        m_parameters(std::move(params))
    {
    }

    http_request::~http_request()
    {
    }

    void http_request::add_header(std::string name, std::string value)
    {
        m_headers.emplace(std::move(name), std::move(value));
    }

    std::string const & http_request::get_header_value(std::string const & name) const
    {
        static std::string const empty_value;

        auto iterator = m_headers.find(name);
        if (iterator != m_headers.cend())
        {
            return iterator->second;
        }

        return empty_value;
    }

    http_protocol const http_request::get_protocol() const
    {
        return m_protocol;
    }

    std::string const & http_request::get_host() const
    {
        return m_host;
    }

    std::string const & http_request::get_url() const
    {
        return m_url;
    }

    std::string const & http_request::get_body() const
    {
        return m_body;
    }

    auto http_request::get_parameters() const -> http_request::parameters_map_type const &
    {
        return m_parameters;
    }

    auto http_request::get_headers() const -> http_request::headers_map_type const &
    {
        return m_headers;
    }
}