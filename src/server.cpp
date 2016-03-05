#include "server.hpp"
#include "library.hpp"
#include "base64.hpp"

#include <algorithm>
#include <memory>

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <mstch/mstch.hpp>

namespace
{
    std::map<std::string, std::string> const parse_parameters(std::string const & contents)
    {
        std::vector<std::string> tokens;
        std::map<std::string, std::string> parameters;

        boost::split(tokens, contents, boost::is_any_of("&"));
        for (auto const & token : tokens)
        {
            auto key_end_pos = token.find('=');
            if (key_end_pos == std::string::npos)
                break;

            auto key = token.substr(0, key_end_pos);
            auto value = token.substr(key_end_pos + 1);

            parameters[key] = value;
        }

        return parameters;
    }

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
    server::server(uint16_t port, json11::Json const & users, manga_library * const lib) :
        m_port(port),
        m_users(users),
        m_library(lib),
        m_app(),
        m_sessions()
    {
        crow::logger::setLogLevel(crow::LogLevel::CRITICAL);

        CROW_ROUTE(m_app, "/mangapp").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie("session_id");

            if (is_authenticated(session_id) == false)
            {
                // Not authenticated yet, so send the login page
                return crow::response(read_file_contents("../static/html/login.html"));
            }
            else
            {
                // We're authenticated, so we can send index.html
                auto index_template(read_file_contents("../static/html/index-template.html"));

                mstch::map context(m_library->get_list_context());

                return crow::response(mstch::render(index_template, context));
            }
        });

        CROW_ROUTE(m_app, "/mangapp/login").methods(crow::HTTPMethod::POST)
            ([this](crow::request const & request) -> crow::response
        {         
            auto const & users_array = m_users.array_items();
            auto parameters(parse_parameters(request.body));

            std::string user_password = parameters["username"] + ":" + parameters["password"];
            bool authenticated = std::any_of(users_array.cbegin(), users_array.cend(),
                [&user_password](json11::Json const & json) -> bool
            {
                return json.string_value() == user_password;
            });

            if (authenticated == true)
            {
                auto session_id = generate_session_id();
                m_app.get_context<crow::CookieParser>(request).set_cookie("session_id", session_id);
                m_sessions.emplace_back(session_id);

                return crow::response(200);
            }
            else
                return crow::response(401);
            
        });

        CROW_ROUTE(m_app, "/mangapp/static/<string>/<string>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, std::string type_str, std::string name_str) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie("session_id");
            if (is_authenticated(session_id) == false)
                return crow::response(401);

            auto file_contents(read_file_contents(std::string("../static/") + type_str + "/" + name_str));
            if (file_contents.empty() != true)
                return crow::response(file_contents);
            else
                return crow::response(404);
        });

        // Route for getting the list of manga
        //CROW_ROUTE(m_app, "/mangapp/list").methods(crow::HTTPMethod::GET)
        //    ([this](crow::request const & request) -> crow::response
        //{
        //    auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie("session_id");
        //    if (is_authenticated(session_id) == false)
        //        return crow::response(401);

        //    auto const mangalist_json = m_library->get_list().dump();
        //    auto response = crow::response(mangalist_json);
        //    response.set_header("Content-Type", "application/json");

        //    return response;
        //});

        // Route for getting the thumbnail of a manga
        CROW_ROUTE(m_app, "/mangapp/thumbnail/<uint>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, size_t key) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie("session_id");
            if (is_authenticated(session_id) == false)
                return crow::response(401);

            auto thumbnail_data(m_library->get_thumbnail(key, true));
            if (thumbnail_data.empty() == true)
            {
                return crow::response(read_file_contents("../static/img/unknown.jpg"));
            }

            return crow::response(thumbnail_data);
        });

        // Route for retrieving mangaupdates details for a manga
        CROW_ROUTE(m_app, "/mangapp/details/<uint>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, crow::response & response, size_t key) -> void
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie("session_id");
            if (is_authenticated(session_id) == false)
            {
                response.code = 401;
                response.end();
                return;
            }
          
            m_library->get_manga_details(key,
                [this, &response, key](mstch::map && context, bool success) -> void
            {
                if (response.is_alive() == true)
                {
                    if (success == true)
                    {
                        auto files_context(std::move(m_library->get_files_context(key)));
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

            //m_library->get_manga_details(key,
            //    [this, &response](json11::Json const & json) -> void
            //{
            //    // Ensure the response is still valid
            //    if (response.is_alive() == true)
            //    {
            //        response.set_header("Content-Type", "application/json");
            //        // Write our JSON data and signal that we have finished writing the response so it can be sent
            //        response.write(json.dump());
            //        response.end();
            //    }
            //});
        });

        // Route for getting the files of a manga
        //CROW_ROUTE(m_app, "/mangapp/files/<uint>").methods(crow::HTTPMethod::GET)
        //    ([this](crow::request const & request, size_t key) -> crow::response
        //{
        //    auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie("session_id");
        //    if (is_authenticated(session_id) == false)
        //        return crow::response(401);

        //    auto const files_json = m_library->get_files(key);
        //    if (files_json.array_items().size() > 0)
        //    {
        //        auto response = crow::response(files_json.dump());
        //        response.set_header("Content-Type", "application/json");
        //        return response;
        //    }
        //    else
        //        return crow::response(404);
        //});
        //}); 

        CROW_ROUTE(m_app, "/mangapp/reader/<uint>/<uint>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, size_t manga_key, size_t file_key) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie("session_id");
            if (is_authenticated(session_id) == false)
                return crow::response(401);

            auto reader_template(read_file_contents("../static/html/reader-template.html"));
            auto context(m_library->get_reader_context(manga_key, file_key));

            return crow::response(mstch::render(reader_template, context));
        });

        // Route for getting an image from an archive
        CROW_ROUTE(m_app, "/mangapp/image/<uint>/<uint>/<uint>").methods(crow::HTTPMethod::GET)
            ([this](crow::request const & request, size_t manga_key, size_t file_key, size_t index) -> crow::response
        {
            auto const & session_id = m_app.get_context<crow::CookieParser>(request).get_cookie("session_id");
            if (is_authenticated(session_id) == false)
                return crow::response(401);

            auto const image_contents = m_library->get_image(manga_key, file_key, index);
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
        m_app.port(m_port).name("Mangapp/0.1").multithreaded().run();
    }

    bool const server::is_authenticated(std::string const & session_id)
    {
        return std::find(m_sessions.cbegin(), m_sessions.cend(), session_id) != m_sessions.cend();
    }
}