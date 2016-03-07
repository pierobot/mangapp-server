#ifndef MANGAPP_SERVER_H
#define MANGAPP_SERVER_H

#include "manga_library.hpp"

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

#include <json11/json11.hpp>

namespace mangapp
{
    class server
    {
    public:
        server(uint16_t port, json11::Json const & json_settings, manga_library * const lib);

        virtual ~server();

        void start();
    protected:
    private:
        crow::Crow<crow::CookieParser> m_app;
        std::vector<std::string> m_sessions;
        uint16_t m_port;
        json11::Json const & m_users;
        json11::Json const & m_tls_ssl;
        manga_library * m_library;

        bool const is_authenticated(std::string const & session_id);
    };
}

#endif
