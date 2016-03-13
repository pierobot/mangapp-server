#ifndef MANGAPP_SERVER_H
#define MANGAPP_SERVER_H

#include <cstdint>
#include <string>
#include <vector>

#ifdef _WIN32
// Windows headers define the following and will cause compile errors if not undef'd
#   undef ERROR
#   undef DELETE
#endif

#include <crow/crow.h>
#include <crow/middleware.h>

namespace json11
{
    class Json;
}

namespace mangapp
{
    class manga_library;

    class server
    {
    public:
        server(uint16_t port, json11::Json const & json_settings, manga_library & lib);

        virtual ~server();

        void start();
    protected:
    private:
        uint16_t m_port;
        manga_library & m_library;
        crow::Crow<crow::CookieParser> m_app;
        json11::Json const & m_users;
        json11::Json const & m_tls_ssl;
        std::vector<std::string> m_sessions;

        bool const is_authenticated(std::string const & session_id);
    };
}

#endif
