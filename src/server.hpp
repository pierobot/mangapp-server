#ifndef MANGAPP_SERVER_H
#define MANGAPP_SERVER_H

#include "manga_library.hpp"

#include <cstdint>

#ifdef _WIN32
// Windows headers define the following and will cause compile errors if not undef'd
#   undef ERROR
#   undef DELETE
#endif

#include <crow/crow.h>
#include <crow/json.h>

namespace mangapp
{
    class server
    {
    public:
        server(uint16_t port, manga_library * const lib);

        virtual ~server();

        void start();
        void stop();
    protected:
    private:
        crow::SimpleApp m_app;
        uint16_t m_port;
        manga_library * m_library;
    };
}

#endif
