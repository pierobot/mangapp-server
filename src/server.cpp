#include "server.hpp"
#include "manga_library.hpp"

#include <algorithm>
#include <memory>

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <mstch/mstch.hpp>

namespace
{
    static std::string const session_cookie_str = "session_id";

    std::string const generate_session_id()
    {
        std::string session_id;
        std::string characters("abcdefghijklmnopqrstuvwxyz"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "1234567890");

        session_id.resize(64);
        boost::random::random_device rng;
        boost::random::uniform_int_distribution<> distribution(0, characters.size() - 1);

        std::generate(session_id.begin(), session_id.end(),
            [&]() -> char
        {
            return characters[distribution(rng)];
        });

        return session_id;
    }

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
    server::server(uint16_t port, json11::Json const & json_settings, manga_library & library) :
        m_port(port),
        m_users(json_settings["users"]),
        m_tls_ssl(json_settings["tls/ssl"]),
        m_library(library),
        m_app(),
        m_sessions()
    {
        crow::logger::setLogLevel(crow::LogLevel::CRITICAL);

        // Main route
        CROW_ROUTE(m_app, "/mangapp").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request) -> crow::response
        {
            auto s = request.get_header_value("id");
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie(session_cookie_str);

            if (is_authenticated(session_id) == false)
            {
                // Not authenticated yet, so send the login page
                return crow::response(read_file_contents("../static/html/login.html"));
            }
            else
            {
                // We're authenticated, so we can send index.html
                auto index_template(read_file_contents("../static/html/index-template.html"));

                mstch::map context(m_library.get_list_context());

                return crow::response(mstch::render(index_template, context));
            }
        });

        // Route for authentication
        CROW_ROUTE(m_app, "/mangapp/login").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request) -> crow::response
        {         
            auto const & users_array = m_users.array_items();
            
            std::string username(request.url_params.get("username"));
            std::string password(request.url_params.get("password"));
            std::string user_password(username + ":" + password);

            bool authenticated = std::any_of(users_array.cbegin(), users_array.cend(),
                [&user_password](json11::Json const & json) -> bool
            {
                return json.string_value() == user_password;
            });

            if (authenticated == true)
            {
                auto session_id = generate_session_id();
                m_app.get_context<crow::CookieParser>(request).set_cookie(session_cookie_str, session_id);
                m_sessions.emplace_back(session_id);

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
            if (is_authenticated(session_id) == false)
                return crow::response(401);

            auto thumbnail_data(m_library.get_thumbnail(key, true));
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
            if (is_authenticated(session_id) == false)
            {
                response.code = 401;
                response.end();
                return;
            }

            m_library.get_details(key,
                [this, &response, key](mstch::map && context, bool success) -> void
            {
                if (response.is_alive() == true)
                {
                    if (success == true)
                    {
                        auto files_context(std::move(m_library.get_files_context(key)));
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
            if (is_authenticated(session_id) == false)
                return crow::response(401);

            auto reader_template(read_file_contents("../static/html/reader-template.html"));
            auto context(m_library.get_reader_context(manga_key, file_key));

            return crow::response(mstch::render(reader_template, context));
        });

        // Route for getting an image from an archive
        CROW_ROUTE(m_app, "/mangapp/image/<uint>/<uint>/<uint>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, size_t manga_key, size_t file_key, size_t index) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie(session_cookie_str);
            if (is_authenticated(session_id) == false)
                return crow::response(401);

            auto const image_contents = m_library.get_image(manga_key, file_key, index);
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

    bool const server::is_authenticated(std::string const & session_id)
    {
        return std::find(m_sessions.cbegin(), m_sessions.cend(), session_id) != m_sessions.cend();
    }
}