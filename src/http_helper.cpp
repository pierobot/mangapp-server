#include "http_helper.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>

#include <zlib.h>

namespace mangapp
{
    static std::string const content_length_str = "Content-Length: ";
    static std::string const transfer_encoding_str = "Transfer-Encoding: ";

    http_helper::http_helper() :
        m_io_service(),
        m_infinite_work(new boost::asio::io_service::work(m_io_service))
    {
        m_is_running = true;

        m_thread = std::thread([this](void)
        {
            while (m_is_running == true)
            {
                m_io_service.run();
                //std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }

    http_helper::~http_helper()
    {
        m_infinite_work.reset();
        m_is_running = false;

        m_io_service.stop();
        m_thread.join();
    }

    static std::string const zlib_inflate(void const * deflated_buffer, unsigned int deflated_size)
    {
        std::string inflated_buffer;
        Bytef tmp[8192];

        z_stream zstream{ 0 };
        zstream.avail_in = deflated_size;
        // Nasty const_cast but zlib won't alter its contents
        zstream.next_in = const_cast<Bytef z_const *>(reinterpret_cast<Bytef const *>(deflated_buffer));

        // Initialize with automatic header detection, for gzip support
        if (inflateInit2(&zstream, MAX_WBITS | 32) == Z_OK)
        {
            do
            {
                zstream.avail_out = sizeof(tmp);
                zstream.next_out = &tmp[0];

                auto ret = inflate(&zstream, Z_NO_FLUSH);

                if (ret == Z_OK)
                {
                    std::copy(&tmp[0], &tmp[8192 - zstream.avail_out], std::back_inserter(inflated_buffer));
                }

            } while (zstream.avail_out == 0);
        }

        return inflated_buffer;
    }

    void http_helper::http_get_async(std::string const & host,
        std::string const & url,
        on_ready_function on_ready,
        on_error_function on_error)
    {
        using namespace boost::asio;

        ip::tcp::resolver resolver(m_io_service);
        ip::tcp::resolver::query query(ip::tcp::v4(), host, "80");
        ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        socket_pointer socket(new ip::tcp::socket(m_io_service));
        string_pointer contents(new std::string());
        context_pointer context(new http_context());

        context->on_error = on_error;
        context->on_ready = on_ready;
        context->host = host;
        context->url = url;
        // Since these calls are asynchronous, we must use copy-by-value in the lambda's captures or the 
        // variables could, and most likely will, be in an invalid state.
        async_connect(*socket, iterator,
            [this, socket, context](boost::system::error_code const & error, ip::tcp::resolver::iterator iterator)
        {
            if (error != 0)
                context->on_error(error.message());
            else
                on_connect(socket, context);
        });
    }

    void http_helper::on_connect(socket_pointer socket, context_pointer context)
    {
        // Build and send the request header
        context->contents = build_request_header(context->host, context->url);
        auto buffer = boost::asio::buffer(&context->contents[0], context->contents.size());
        async_write(*socket, buffer,
            [this, socket, context](boost::system::error_code const & error, size_t bytes_written)
        {
            if (error != 0)
                context->on_error(error.message());
            else
                on_write(socket, context, bytes_written);
        });
    }

    void http_helper::on_read(socket_pointer socket, context_pointer context, size_t bytes_read)
    {
        if (context->content_length == 0 && context->is_chunked == false)
        {
            context->contents = get_header_from_stream(context->stream, bytes_read);
            context->content_length = get_content_length(context->contents);
            if (context->content_length == 0)
            {
                context->is_chunked = is_chunked(context->contents);
                if (context->is_chunked == false)
                {
                    context->on_error("Malformed HTTP response: No Content-Length nor chunked Transfer-Encoding.");
                    return;
                }
            }

            context->contents = get_contents_from_stream(context->stream, bytes_read, context->is_chunked ? 0 : context->content_length);
            if (context->is_chunked == true)
            {
                auto chunk_length = get_chunk_length(context->contents);
                if (chunk_length == 0)
                {
                    context->on_error("Malformed HTTP response: Unable to find length of chunk.");
                    return;
                }

                context->chunk_length = chunk_length;
                context->contents = erase_chunk_length(context->contents);
            }

            context->bytes_transferred += context->contents.size();
            context->contents.resize(context->content_length);
            auto buffer = boost::asio::buffer(&context->contents[context->bytes_transferred], context->content_length - context->bytes_transferred);
            async_read(*socket, buffer,
                [this, socket, context](boost::system::error_code const & error, size_t bytes_read)
            {
                if (error != 0)
                    context->on_error(error.message());
                else
                    on_read(socket, context, bytes_read);
            });
        }
        else
        {
            context->bytes_transferred += bytes_read;
            // Do we have all the data?
            if (context->bytes_transferred < context->content_length)
            {
                // No, issue another read for the remaining content
                auto buffer = boost::asio::buffer(&context->contents[context->bytes_transferred],
                    context->content_length - context->bytes_transferred);
                async_read(*socket, buffer,
                    [this, socket, context](boost::system::error_code const & error, size_t bytes_read)
                {
                    if (error != 0)
                        context->on_error(error.message());
                    else
                        on_read(socket, context, bytes_read);
                });
            }
            else
            {
                // Yes, inflate the gzip/deflate encoded data
                auto inflated_data = zlib_inflate(&context->contents[0], context->contents.size());
                if (inflated_data.empty() == false && context->on_ready != nullptr)
                    context->on_ready(inflated_data);
            }
        }
    }

    void http_helper::on_write(socket_pointer socket, context_pointer context, size_t bytes_written)
    {
        // We no longer need the request header
        context->contents.clear();

        async_read_until(*socket, context->stream, "\r\n\r\n",
            [this, socket, context](boost::system::error_code const & error, size_t bytes_read)
        {
            if (error != 0)
                context->on_error(error.message());
            else
                on_read(socket, context, bytes_read);
        });
    }

    std::string const http_helper::get_header_from_stream(boost::asio::streambuf & streambuf, size_t bytes_read) const
    {
        std::string header;
        auto buffers = streambuf.data();

        header.reserve(streambuf.size());
        std::copy(boost::asio::buffers_begin(buffers),
                  boost::asio::buffers_begin(buffers) + bytes_read,
                  std::back_inserter(header));

        return header;
    }

    std::string const http_helper::get_contents_from_stream(boost::asio::streambuf & streambuf,
        size_t bytes_read,
        size_t expected_size /* = 0 */) const
    {
        std::string contents;
        auto buffers = streambuf.data();

        if (expected_size > 0)
            contents.reserve(expected_size);

        std::copy(boost::asio::buffers_begin(buffers) + bytes_read,
                  boost::asio::buffers_end(buffers),
                  std::back_inserter(contents));

        return contents;
    }

    std::string & http_helper::erase_chunk_length(std::string & contents) const
    {
        auto start_pos = contents.find_first_not_of("\r\n");
        if (start_pos != std::string::npos)
        {
            auto end_pos = contents.find_first_of("\r\n");
            if (end_pos != std::string::npos)
            {
                end_pos += 2;
                contents.erase(start_pos, end_pos);
            }
        }

        return contents;
    }

    std::string const http_helper::build_request_header(std::string const & host, std::string const & url) const
    {
        return "GET " + url + " HTTP/1.1\r\n"
               "Accept: text/html\r\n"
               "Accept-Encoding: gzip, deflate\r\n"
               "DNT: 1\r\n"
               "Host: " + host + "\r\n"
               "Connection: close\r\n"
               "Pragma: no-cache\r\n"
               "Cache-Control: no-cache\r\n"
               "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36\r\n"
               "DNT: 1\r\n"
               "\r\n";
    }

    size_t const http_helper::get_header_length(std::string const & headers) const
    {
        auto length = 0;
        auto start_pos = headers.find("\r\n\r\n");
        if (start_pos != std::string::npos)
        {
            length = start_pos + 4;
        }

        return length;
    }

    size_t const http_helper::get_content_length(std::string const & headers) const
    {
        auto length = 0;
        auto start_pos = headers.find(content_length_str);

        if (start_pos != std::string::npos)
        {
            start_pos += content_length_str.length();
            auto end_pos = headers.find("\r\n", start_pos);

            length = std::stoul(headers.substr(start_pos, end_pos - start_pos));
        }

        return length;
    }

    bool http_helper::is_chunked(std::string const & headers) const
    {
        bool flag = false;
        auto start_pos = headers.find(transfer_encoding_str);

        if (start_pos != std::string::npos)
        {
            start_pos += transfer_encoding_str.length();
            auto end_pos = headers.find("\r\n", start_pos);

            flag = headers.substr(start_pos, end_pos - start_pos) == "chunked";
        }

        return flag;
    }

    size_t const http_helper::get_chunk_length(std::string const & contents) const
    {
        size_t length = 0;

        auto end_pos = contents.find("\r\n");
        if (end_pos != std::string::npos)
        {
            length = std::stoul(contents.substr(0, end_pos), nullptr, 16);
        }

        return length;
    }
}
