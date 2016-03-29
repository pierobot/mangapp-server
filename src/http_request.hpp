#ifndef MANGAPP_HTTP_REQUEST_HPP
#define MANGAPP_HTTP_REQUEST_HPP

#include <functional>
#include <initializer_list>
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

    class http_request
    {
    public:
        typedef std::initializer_list<std::pair<const std::string, std::string>> parameters_init_type;
        typedef std::map<std::string, std::string> parameters_map_type;
        typedef std::unordered_multimap<std::string, std::string> headers_map_type;

        http_request(http_protocol protocol, std::string host, std::string url, std::string body, parameters_init_type params = {});
        ~http_request();

        void add_header(std::string name, std::string value);
        std::string const & get_header_value(std::string const & name) const;

        http_protocol const get_protocol() const;
        std::string const & get_host() const;
        std::string const & get_url() const;
        std::string const & get_body() const;

        parameters_map_type const & get_parameters() const;
        headers_map_type const & get_headers() const;
    protected:
    private:
        http_protocol const m_protocol;
        std::string const m_host;
        std::string const m_url;
        std::string const m_body;
        parameters_map_type m_parameters;
        headers_map_type m_headers;
    };
}

#endif