#ifndef MANGAPP_SERVER_H
#define MANGAPP_SERVER_H

#include "manga_library.hpp"
#include "users.hpp"

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

namespace mangapp
{
    class server
    {
    public:
        server(json11::Json const & settings_json);

        virtual ~server();

        void start();
    protected:
    private:
        uint16_t m_port;
        users m_users;
        manga_library m_manga_library;
        crow::Crow<crow::CookieParser> m_app;
        json11::Json const & m_tls_ssl;
    };
}

#endif
