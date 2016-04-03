#include "http_request.hpp"

namespace mangapp
{
    http_request::http_request(http_protocol protocol, http_action action, std::string host, std::string url) :
        m_protocol(protocol),
        m_action(action),
        m_host(std::move(host)),
        m_url(std::move(url)),
        m_body(),
        m_parameters()
    {
    }

    http_request::~http_request()
    {
    }

    void http_request::add_parameter(std::string key, std::string value)
    {
        m_parameters.emplace(std::move(key), std::move(value));
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

    void http_request::set_body(std::string body)
    {
        m_body = std::move(body);
    }

    http_protocol const http_request::get_protocol() const
    {
        return m_protocol;
    }

    http_action const http_request::get_action() const
    {
        return m_action;
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