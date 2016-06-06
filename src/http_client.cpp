#include "http_client.hpp"
#include "http_utility.hpp"

#include <fstream>

#include <zlib.h>

namespace mangapp
{
    http_client::http_client(http_version version, uint8_t max_sockets) :
        m_version(version),
        m_is_running(true),
        m_max_sockets(max_sockets),
        m_socket_count(0),
        m_thread(),
        m_mutex_work(),
        m_pending_work(),
        m_io_service(),
        m_infinite_work(new boost::asio::io_service::work(m_io_service))
    {
        
        m_thread = std::thread([this]()
        {
            while (m_is_running == true)
            {
                m_io_service.poll();
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                // Do any pending socket work
                m_io_service.post([this]() -> void
                {
                    std::lock_guard<std::mutex> lock(m_mutex_work);
                    if (m_pending_work.size() > 0)
                    {
                        if (m_socket_count < m_max_sockets)
                        {
                            auto work_function = m_pending_work.front();
                            m_pending_work.pop();
                            work_function();
                        }
                    }
                });
            }
        });
    }

    http_client::~http_client()
    {
        m_is_running = false;
        m_io_service.stop();
        m_thread.join();
    }

    bool http_client::default_verify(bool preverified, boost::asio::ssl::verify_context & context)
    {
        return true;
    }

    std::string const http_client::to_string(http_request const & request) const
    {
        std::string url_and_params = request.get_url() + std::string("?");

        for (auto const & parameter : request.get_parameters())
        {
            url_and_params += std::string("&") + parameter.first + std::string("=") + parameter.second;
        }

        std::string version_str;
        switch (m_version)
        {
        case mangapp::http_version::v1_0:
            version_str = "1.0";
            break;
        case mangapp::http_version::v1_1:
        default:
            version_str = "1.1";
            break;
        }

        std::string headers;
        for (auto const & header : request.get_headers())
        {
            headers += header.first + ": " + header.second + "\r\n";
        }
        
        std::string action;
        switch (request.get_action())
        {
        case http_action::post:
            action = "POST ";
        case http_action::get:
        default:
            action = "GET ";
        }

        std::string buffer = action + url_and_params + " HTTP/" + version_str + "\r\n" +
                             headers + 
                             request.get_body() + "\r\n\r\n";

        return buffer;
    }

    std::string const http_client::get_header_from_stream(boost::asio::streambuf & streambuf, size_t bytes_read) const
    {
        std::string header;
        auto buffers = streambuf.data();

        header.reserve(streambuf.size());
        std::copy(boost::asio::buffers_begin(buffers),
                  boost::asio::buffers_end(buffers),
                  std::back_inserter(header));

        streambuf.consume(streambuf.size());
        
        return header;
    }

    size_t const http_client::get_chunk_length(std::string const & contents) const
    {
        size_t length = 0;

        auto end_pos = contents.find("\r\n");
        if (end_pos != std::string::npos)
        {
            length = std::stoul(contents.substr(0, end_pos), nullptr, 16);
        }

        return length;
    }

    std::string const http_client::get_body_from_stream(boost::asio::streambuf & streambuf,
                                                        size_t bytes_read,
                                                        size_t expected_size /* = 0 */) const
    {
        std::string contents;
        auto buffers = streambuf.data();
        
        if (expected_size > 0)
            contents.reserve(expected_size);

        std::copy(boost::asio::buffers_begin(buffers),
                  boost::asio::buffers_end(buffers),
                  std::back_inserter(contents));

        streambuf.consume(streambuf.size());

        return contents;
    }

    std::string & http_client::erase_chunk_length(std::string & contents) const
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

    std::string const http_client::zlib_inflate(void const * deflated_buffer, unsigned int deflated_size) const
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
                if (ret == Z_OK || ret == Z_STREAM_END)
                {
                    std::copy(&tmp[0], &tmp[8192 - zstream.avail_out], std::back_inserter(inflated_buffer));
                }
                else
                {
                    // Something went wrong with inflate; make sure we return an empty string
                    inflated_buffer.clear();
                    break;
                }

            } while (zstream.avail_out == 0);

            // Free zlib's internal memory
            inflateEnd(&zstream);
        }

        return inflated_buffer;
    }

    void http_client::do_connect_async(request_pointer && request, ready_function on_ready, error_function on_error)
    {
        boost::asio::ip::tcp::resolver resolver(m_io_service);
        std::string port = request->get_protocol() == http_protocol::https ? "443" : "80";
        boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), request->get_host(), port);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        context_pointer context(new http_context());

        context->on_error = on_error;
        context->on_ready = on_ready;
        context->request_ptr = request;

        if (request->get_protocol() == http_protocol::https)
        {
            // Yes, setup our SSL context
            boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23_client);
            //ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
            //ssl_context.set_verify_callback([this](bool preverified, boost::asio::ssl::verify_context & context) -> bool
            //{
            //    return default_verify(preverified, context);
            //});
            // Create the SSL stream that will handle the protocol's messages and encryption/decryption
            // 
            ssl_socket_deleter ssl_deleter(m_socket_count);
            socket_ssl_pointer ssl_socket(new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(m_io_service, ssl_context), ssl_deleter);
            if (ssl_socket != nullptr)
            {
                context->socket_ssl = std::move(ssl_socket);

                boost::asio::async_connect(context->socket_ssl->lowest_layer(), iterator,
                    [this, context](boost::system::error_code const & error, boost::asio::ip::tcp::resolver::iterator iterator)
                {
                    if (error != 0)
                        context->on_error(error.message());
                    else
                        on_connect(context);
                });
            }
        }
        else
        {
            socket_deleter deleter(m_socket_count);
            socket_pointer socket(new boost::asio::ip::tcp::socket(m_io_service), deleter);
            if (socket != nullptr)
            {
                context->socket = std::move(socket);

                boost::asio::async_connect(*context->socket, iterator,
                    [this, context](boost::system::error_code const & error, boost::asio::ip::tcp::resolver::iterator iterator)
                {
                    if (error != 0)
                        context->on_error(error.message());
                    else
                        on_connect(context);
                });
            }
        }
    }

    void http_client::send(request_pointer && request, ready_function on_ready, error_function on_error)
    {
        if (m_socket_count < m_max_sockets)
        {
            do_connect_async(std::move(request), on_ready, on_error);
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_mutex_work);
            m_pending_work.push([this, request, on_ready, on_error]() mutable -> void
            {
                do_connect_async(std::move(request), on_ready, on_error);
            });
        }
    }

    void http_client::on_connect(context_pointer context)
    {
        // Is the request HTTPS?
        if (context->request_ptr->get_protocol() == http_protocol::https)
        {
            // Handle the handshake
            context->socket_ssl->async_handshake(boost::asio::ssl::stream_base::handshake_type::client,
                [this, context](boost::system::error_code const & error)
            {
                if (!error)
                    on_handshake(context);
                else
                    context->on_error(error.message());
            });
        }
        else
        {
            on_handshake(context);
        }
    }

    void http_client::on_handshake(context_pointer context)
    {
        auto const request_str = to_string(*context->request_ptr);
        auto request_buffer = boost::asio::buffer(&request_str[0], request_str.size());

        auto lambda_on_write = [this, context](boost::system::error_code const & error, size_t bytes_written)
        {
            if (!error)
                on_write(context, bytes_written);
            else
                context->on_error(error.message());
        };

        if (context->request_ptr->get_protocol() == http_protocol::https)
            boost::asio::async_write(*context->socket_ssl, request_buffer, lambda_on_write);
        else
            boost::asio::async_write(*context->socket, request_buffer, lambda_on_write);
    }

    void http_client::on_read(context_pointer context, size_t bytes_read)
    {
        auto lambda_on_read = [this, context](boost::system::error_code const & error, size_t bytes_read)
        {
            if (!error)
            {
                on_read(context, bytes_read);
            }
            else
            {
                if ((error.value() & 0xff) == SSL_R_SHORT_READ)
                    on_read(context, bytes_read);
                else
                    context->on_error(error.message());
            }
        };

        if (context->content_length == 0 && context->is_chunked == false)
        {
            // Set up our resposne headers
            auto headers = get_header_from_stream(context->stream, bytes_read);
            if (context->response_ptr == nullptr)
            {
                context->response_ptr = response_pointer(new http_response(std::move(headers)));
                if (context->response_ptr == nullptr)
                {
                    context->on_error("Unable to allocate memory for http_response");
                    return;
                }

                auto const & content_length_str = context->response_ptr->get_header_value("Content-Length");
                if (content_length_str.empty() == false)
                {
                    context->content_length = std::stoull(content_length_str);
                }
                else
                {
                    context->is_chunked = context->response_ptr->get_header_value("Transfer-Encoding") == "chunked";
                    if (context->is_chunked == false)
                    {
                        context->on_error("Malformed response - No Content-Length nor chunked Transfer-Encoding.");
                        return;
                    }
                }

                // Start the initial read for the body
                if (context->is_chunked == true)
                {
                    // Since this is chunked, we can read until we reach \r\n
                    if (context->request_ptr->get_protocol() == http_protocol::https)
                        boost::asio::async_read_until(*context->socket_ssl, context->stream, "\r\n", lambda_on_read);
                    else
                        boost::asio::async_read_until(*context->socket, context->stream, "\r\n", lambda_on_read);
                }
                else
                {
                    if (context->request_ptr->get_protocol() == http_protocol::https)
                        boost::asio::async_read(*context->socket_ssl, context->stream, lambda_on_read);
                    else
                        boost::asio::async_read(*context->socket, context->stream, lambda_on_read);
                }
            }
        }
        else
        {
            context->bytes_transferred += bytes_read;

            if (context->is_chunked == true)
            {
                if (bytes_read > 0)
                {
                    // Do we have an expected chunk length from a previous read?
                    if (context->chunk_length == 0)
                    {
                        // No, get it
                        context->body = get_body_from_stream(context->stream, 0, bytes_read);
                        size_t chunk_str_length = context->body.length();
                        context->chunk_length = get_chunk_length(context->body);
                        context->body = erase_chunk_length(context->body);

                        context->bytes_transferred -= chunk_str_length;
                    }
                    else
                    {
                        context->body += get_body_from_stream(context->stream, 0, bytes_read);
                    }
                }
            }

            // Do we have all the data?
            if (context->bytes_transferred < context->content_length)
            {
                // No, issue another read for the remaining content
                if (context->is_chunked == true)
                {
                    if (context->request_ptr->get_protocol() == http_protocol::https)
                        boost::asio::async_read(*context->socket_ssl, context->stream, lambda_on_read);
                    else
                        boost::asio::async_read(*context->socket, context->stream, lambda_on_read);
                }
                else
                {
                    if (context->request_ptr->get_protocol() == http_protocol::https)
                        boost::asio::async_read(*context->socket_ssl, context->stream, lambda_on_read);
                    else
                        boost::asio::async_read(*context->socket, context->stream, lambda_on_read);
                }
            }
            else
            {
                // We have all the data, but there's a possibility that another chunk follows
                if (context->is_chunked == true && bytes_read > 0)
                {
                    if (context->request_ptr->get_protocol() == http_protocol::https)
                        boost::asio::async_read_until(*context->socket_ssl, context->stream, "\r\n", lambda_on_read);
                    else
                        boost::asio::async_read_until(*context->socket, context->stream, "\r\n", lambda_on_read);

                    return;
                }

                std::string contents;

                if (context->body.empty() == true)
                    context->body = get_body_from_stream(context->stream, 0, bytes_read);

                if (context->response_ptr->get_header_value("Content-Encoding") == "gzip" ||
                    context->response_ptr->get_header_value("Content-Encoding") == "deflate")
                {
                    contents = zlib_inflate(&context->body[0], context->body.size());
                    context->body.clear();
                }
                else
                {
                    contents = std::move(context->body);
                }

                context->time_total = std::chrono::high_resolution_clock::now() - context->time_start;
                context->response_ptr->set_body(std::move(contents));
                // Forward the contents to the callback
                if (context->on_ready != nullptr)
                {
                    context->on_ready(std::move(context->response_ptr));
                }
            }
        }
    }

    void http_client::on_write(context_pointer context, size_t bytes_written)
    {
        auto lambda_on_read = [this, context](boost::system::error_code const & error, size_t bytes_read)
        {
            if (error != 0)
                context->on_error(error.message());
            else
                on_read(context, bytes_read);
        };

        if (context->request_ptr->get_protocol() == http_protocol::https)
            boost::asio::async_read_until(*context->socket_ssl, context->stream, "\r\n\r\n", lambda_on_read);
        else
            boost::asio::async_read_until(*context->socket, context->stream, "\r\n\r\n", lambda_on_read);
    }
}
