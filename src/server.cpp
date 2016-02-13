#include "server.hpp"
#include "library.hpp"
#include "base64.hpp"

#include <memory>

using namespace mangapp;

server::server(uint16_t port, manga_library * const lib) :
    m_port(port), 
    m_library(lib),
    m_app()
{
    crow::logger::setLogLevel(crow::LogLevel::CRITICAL);
    
    // Route for getting the list of manga
    CROW_ROUTE(m_app, "/mangapp/list").methods(crow::HTTPMethod::GET)
        ([this]() -> crow::response
    {
        auto const mangalist_json = m_library->get_list().dump();
        auto response = crow::response(mangalist_json);
        response.set_header("Content-Type", "application/json");
        //response.set_header("Access-Control-Allow-Origin", "*");

        return response;
    });

    // Route for getting the thumbnail of a manga
    CROW_ROUTE(m_app, "/mangapp/thumbnail/<uint>").methods(crow::HTTPMethod::GET)
        ([this](size_t key) -> crow::response
    {
        auto thumbnail_data = m_library->get_thumbnail(key, true);
        if (thumbnail_data.empty() == true)
            return crow::response(404);

        auto response = crow::response(thumbnail_data);
        //response.set_header("Content-Type", "image/jpeg");

        return response;
    });

    // Route for retrieving mangaupdates details for a manga
    CROW_ROUTE(m_app, "/mangapp/details/<uint>").methods(crow::HTTPMethod::GET)
        ([this](crow::request const &, crow::response & response, size_t key) -> void
    {
        m_library->get_manga_details(key,
            [this, &response](json11::Json const & json) -> void
        {
            // Ensure the response is still valid
            if (response.is_alive() == true && response.is_completed() == false)
            {
                response.set_header("Content-Type", "application/json");
                // Write our JSON data and signal that we have finished writing the response so it can be sent
                response.write(json.dump());
                response.end();
            }
        });
    });

    // Route for getting the files of a manga
    CROW_ROUTE(m_app, "/mangapp/files/<uint>").methods(crow::HTTPMethod::GET)
        ([this](size_t key) -> crow::response
    {
        auto const files_json = m_library->get_files(key);
        if (files_json.array_items().size() > 0)
        {
            auto response = crow::response(files_json.dump());
            response.set_header("Content-Type", "application/json");
            return response;
        }
        else
            return crow::response(404);
    });

    // Route for getting an image from an archive
    CROW_ROUTE(m_app, "/mangapp/reader/<uint>/<uint>/<uint>").methods(crow::HTTPMethod::GET)
        ([this](size_t manga_key, size_t file_key, size_t index) -> crow::response
    {
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
