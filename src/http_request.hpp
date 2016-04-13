#ifndef MANGAPP_HTTP_REQUEST_HPP
#define MANGAPP_HTTP_REQUEST_HPP

#include <map>
#include <string>
#include <unordered_map>

namespace mangapp
{
    enum class http_protocol
    {
        http,
        https
    };

    enum class http_action
    {
        get,
        post
    };

    class http_request
    {
    public:
        typedef std::map<std::string, std::string> parameters_map_type;
        typedef std::unordered_multimap<std::string, std::string> headers_map_type;

        http_request(http_protocol protocol, http_action action, std::string host, std::string url);
        ~http_request();

        void add_parameter(std::string key, std::string value);
        void add_header(std::string name, std::string value);
        std::string const & get_header_value(std::string const & name) const;
        void set_body(std::string body);

        http_protocol const get_protocol() const;
        http_action const get_action() const;
        std::string const & get_host() const;
        std::string const & get_url() const;
        std::string const & get_body() const;

        parameters_map_type const & get_parameters() const;
        headers_map_type const & get_headers() const;
    protected:
    private:
        http_protocol const m_protocol;
        http_action const m_action;
        std::string const m_host;
        std::string const m_url;
        std::string m_body;
        parameters_map_type m_parameters;
        headers_map_type m_headers;
    };
}

#endif