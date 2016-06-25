#ifndef MANGAPP_MIDDLEWARE_DEFLATE
#define MANGAPP_MIDDLEWARE_DEFLATE

#include <functional>

#include <crow/http_request.h>
#include <crow/http_response.h>

#include <zlib.h>

extern void console_write_line(std::string const & function_name, std::string const & message);

static std::string compress_string(std::string const & str, std::function<void(std::string const &)> on_error)
{
    std::string compressed_str;

    z_stream stream{};

    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15, 8, Z_DEFAULT_STRATEGY) == Z_OK)
    {
        uint8_t buffer[8192];

        stream.avail_in = str.size();
        stream.next_in = const_cast<Bytef z_const *>(reinterpret_cast<Bytef const *>(str.c_str()));

        do
        {
            stream.avail_out = 8192;
            stream.next_out = &buffer[0];

            if (::deflate(&stream, Z_FULL_FLUSH) == Z_OK)
            {
                std::copy(&buffer[0], &buffer[8192 - stream.avail_out], std::back_inserter(compressed_str));
            }
            else
            {
                on_error("Unable to compress a chunk of response data. Possible corrupt data.");
                
                compressed_str.clear();
                break;
            }

        } while (stream.avail_in > 0);

        deflateEnd(&stream);
    }
    else
        on_error("Unable to compress response data.");

    return compressed_str;
}

namespace mangapp
{
    namespace middleware
    {
        struct deflate
        {
            struct context
            {
                bool compress = true;
            };

            void before_handle(crow::request & request, crow::response & response, context & ctx) {}

            void after_handle(crow::request & request, crow::response & response, context & ctx)
            {
                if (ctx.compress == false)
                    return;

                std::string compressed_body = std::move(compress_string(response.body,
                    [](std::string const & message) -> void
                {
                    console_write_line("middleware::deflate::after_handle", message);
                }));

                if (compressed_body.empty() == false)
                {
                    response.body = std::move(compressed_body);
                    response.add_header("Content-Encoding", "deflate");
                }
            }
        };
    }
}

#endif