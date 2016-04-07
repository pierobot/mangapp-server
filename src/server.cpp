#include "server.hpp"
#include "manga_library.hpp"
#include "users.hpp"

#include <algorithm>
#include <memory>

#include <mstch/mstch.hpp>

namespace
{
    static std::string const session_cookie_str = "session_id";

    std::string read_file_contents(std::string const & path)
    {
        std::string contents;
        std::ifstream file_stream(path, std::ios::binary);

        if (file_stream.good() == true)
        {
            contents = std::string(std::istreambuf_iterator<char>(file_stream), (std::istreambuf_iterator<char>()));
        }

        return contents;
    }
}

namespace mangapp
{
    server::server(uint16_t port, json11::Json const & json_settings, users & usrs, manga_library & library) :
        m_port(port),
        m_users(usrs),
        m_library(library),
        m_app(),
        m_tls_ssl(json_settings["tls/ssl"])
    {
        crow::logger::setLogLevel(crow::LogLevel::CRITICAL);

        // Main route
        CROW_ROUTE(m_app, "/mangapp").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie(session_cookie_str);

            if (m_users.is_authenticated(session_id) == false)
            {
                // Not authenticated yet, so send the login page
                return crow::response(read_file_contents("../static/html/login.html"));
            }
            else
            {
                // We're authenticated, so we can send index.html
                auto index_template(read_file_contents("../static/html/index-template.html"));

                mstch::map context(m_library.get_list_context(session_id));

                return crow::response(mstch::render(index_template, context));
            }
        });

        // Route for authentication
        CROW_ROUTE(m_app, "/mangapp/login").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request) -> crow::response
        {         
            std::string username = request.url_params.get("username");
            std::string password = request.url_params.get("password");

            std::string session_id;
            bool authenticated = m_users.authenticate(username, password, session_id);
            if (authenticated == true)
            {
                m_app.get_context<crow::CookieParser>(request).set_cookie(session_cookie_str, session_id);

                return crow::response(200);
            }
            else
                return crow::response(401);
            
        });

        CROW_ROUTE(m_app, "/mangapp/logout").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie(session_cookie_str);
            if (m_users.is_authenticated(session_id) == true)
            {
                m_users.logout(session_id);

                return crow::response(200);
            }
            else
                return crow::response(401);
        });

        // Route for serving static web content
        CROW_ROUTE(m_app, "/mangapp/static/<string>/<string>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, std::string type_str, std::string name_str) -> crow::response
        {
            auto file_contents(read_file_contents(std::string("../static/") + type_str + "/" + name_str));
            if (file_contents.empty() != true)
                return crow::response(file_contents);
            else
                return crow::response(404);
        });

        // Route for getting the thumbnail of a manga/comic
        CROW_ROUTE(m_app, "/mangapp/thumbnail/<uint>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, size_t key) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie(session_cookie_str);
            if (m_users.is_authenticated(session_id) == false)
                return crow::response(401);

            auto thumbnail_data(m_library.get_thumbnail(session_id, key));
            if (thumbnail_data.empty() == true)
            {
                return crow::response(read_file_contents("../static/img/unknown.jpg"));
            }

            return crow::response(thumbnail_data);
        });

        // Route for getting the details page for a manga/comic
        CROW_ROUTE(m_app, "/mangapp/details/<uint>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, crow::response & response, size_t key) -> void
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie(session_cookie_str);
            if (m_users.is_authenticated(session_id) == false)
            {
                response.code = 401;
                response.end();
                return;
            }

            m_library.get_details(key,
                [this, &response, session_id, key](mstch::map && context, bool success) -> void
            {
                if (response.is_alive() == true)
                {
                    if (success == true)
                    {
                        auto files_context(std::move(m_library.get_files_context(session_id, key)));
                        context.insert(files_context.begin(), files_context.end());

                        auto details_template(read_file_contents("../static/html/details-template.html"));

                        response.write(mstch::render(details_template, context));
                    }
                    else
                    {
                        response.code = 502;
                    }

                    response.end();
                }
            });
        });

        // Route for getting the reader page for a manga/comic archive
        CROW_ROUTE(m_app, "/mangapp/reader/<uint>/<uint>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, size_t manga_key, size_t file_key) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie(session_cookie_str);
            if (m_users.is_authenticated(session_id) == false)
                return crow::response(401);

            auto reader_template(read_file_contents("../static/html/reader-template.html"));
            auto context(m_library.get_reader_context(session_id, manga_key, file_key));

            return crow::response(mstch::render(reader_template, context));
        });

        // Route for getting an image from an archive
        CROW_ROUTE(m_app, "/mangapp/image/<uint>/<uint>/<uint>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, size_t manga_key, size_t file_key, size_t index) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie(session_cookie_str);
            if (m_users.is_authenticated(session_id) == false)
                return crow::response(401);

            auto const image_contents = m_library.get_image(session_id, manga_key, file_key, index);
            if (image_contents.empty() == false)
            {
                auto response = crow::response(image_contents);
                return response;
            }
            else
                return crow::response(404);
        });
    }

    server::~server()
    {
    }

    void server::start()
    {
        auto const & crt_file = m_tls_ssl["crt-file"].string_value();
        auto const & key_file = m_tls_ssl["key-file"].string_value();

        crow::ssl_context_t ssl_context(crow::ssl_context_t::sslv23);

        ssl_context.use_certificate_file(crt_file, crow::ssl_context_t::file_format::pem);
        ssl_context.use_rsa_private_key_file(key_file, crow::ssl_context_t::file_format::pem);

        m_app.port(m_port).name("Mangapp/0.1").ssl(std::move(ssl_context)).multithreaded().run();
    }
}