#ifndef MANGAPP_MIDDLEWARE_COMPRESSION
#define MANGAPP_MIDDLEWARE_COMPRESSION

#include <functional>

#include <crow/http_request.h>
#include <crow/http_response.h>

#include <zlib.h>

extern void console_write_line(std::string const & function_name, std::string const & message);

enum compression_algorithm
{
    DEFLATE = 15,
    GZIP = 15|16,
};

static std::string compress_string(std::string const & str, compression_algorithm algo)
{
    std::string compressed_str;

    z_stream stream{};

    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, algo, 8, Z_DEFAULT_STRATEGY) == Z_OK)
    {
        // If all the compression is to be done in a single step, deflate() may be called with 
        // flush set to Z_FINISH immediately after the stream has been initialized if avail_out is set to at least the value returned by deflateBound().
        
        compressed_str.resize(deflateBound(&stream, str.size()));
        
        stream.avail_in = str.size();
        stream.next_in = const_cast<Bytef z_const *>(reinterpret_cast<Bytef const *>(str.c_str()));

        stream.avail_out = compressed_str.size();
        stream.next_out = reinterpret_cast<Bytef *>(&compressed_str[0]);

        int code = ::deflate(&stream, Z_FINISH);
        if (code != Z_STREAM_END)
        {
            compressed_str.clear();
        }
        else
        {
            compressed_str = std::string(compressed_str.cbegin(), compressed_str.cbegin() + stream.total_out);
        }

        deflateEnd(&stream);
    }


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

                std::string compressed_body = compress_string(response.body, compression_algorithm::DEFLATE);

                if (compressed_body.empty() == false)
                {
                    response.body = std::move(compressed_body);
                    response.add_header("Content-Encoding", "deflate");
                }
                else
                {
                    console_write_line("middleware::deflate::after_handle", "Compression failed. Defaulting to uncompressed data for response to: " + request.url);
                }
            }
        };

        struct gzip
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

                std::string compressed_body = compress_string(response.body, compression_algorithm::GZIP);

                if (compressed_body.empty() == false)
                {
                    response.body = std::move(compressed_body);
                    response.add_header("Content-Encoding", "gzip");
                }
                else
                {
                    console_write_line("middleware::gzip::after_handle", "Compression failed. Defaulting to uncompressed data for response to: " + request.url);
                }
            }

        };
    }
}

#endif