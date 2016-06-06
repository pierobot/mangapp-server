#ifndef MANGAPP_HTTP_CLIENT_HPP
#define MANGAPP_HTTP_CLIENT_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <queue>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "http_request.hpp"
#include "http_response.hpp"

namespace mangapp
{
    enum class http_version
    {
        v1_0,
        v1_1
    };

    struct http_context
    {
        std::shared_ptr<http_request> request_ptr;
        std::shared_ptr<http_response> response_ptr;

        std::shared_ptr<boost::asio::ip::tcp::socket> socket;
        std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_ssl;

        boost::asio::streambuf stream;

        std::function<void(std::string const&)> on_error;
        std::function<void(std::shared_ptr<http_response> &&)> on_ready;

        bool is_chunked;
        union
        {
            size_t content_length;
            size_t chunk_length;
        };
        size_t bytes_transferred;

        std::string body;
        
        std::chrono::high_resolution_clock::time_point time_start;
        std::chrono::high_resolution_clock::duration time_total;

        http_context() :
            request_ptr(nullptr), response_ptr(nullptr),
            socket(nullptr), socket_ssl(nullptr),
            stream(),
            on_error(nullptr), on_ready(nullptr),
            is_chunked(false),
            content_length(0), bytes_transferred(0),
            body(),
            time_start(std::chrono::high_resolution_clock::now()),
            time_total()
        {
        }
    };

    class socket_deleter
    {
    public:
        socket_deleter(std::atomic<uint8_t> & socket_count) :
            m_socket_count(socket_count)
        {
            ++m_socket_count;
        }

        void operator()(boost::asio::ip::tcp::socket * p)
        {
            if (p != nullptr)
            {
                delete p;

                --m_socket_count;
            }
        }
    private:
        std::atomic<uint8_t> & m_socket_count;
    };

    class ssl_socket_deleter
    {
    public:
        ssl_socket_deleter(std::atomic<uint8_t> & socket_count) :
            m_socket_count(socket_count)
        {
            ++m_socket_count;
        }

        void operator()(boost::asio::ssl::stream<boost::asio::ip::tcp::socket> * p)
        {
            if (p != nullptr)
            {
                delete p;

                --m_socket_count;
            }
        }
    private:
        std::atomic<uint8_t> & m_socket_count;
    };

    class http_client
    {
    public:
        typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_pointer;
        typedef std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_ssl_pointer;
        typedef std::shared_ptr<std::string> string_pointer;
        typedef std::shared_ptr<http_context> context_pointer;
        typedef std::shared_ptr<boost::asio::io_service::work> work_pointer;
        
        typedef std::shared_ptr<http_request> request_pointer;
        typedef std::shared_ptr<http_response> response_pointer;

        typedef std::function<void(std::string const &)> error_function;
        typedef std::function<void(response_pointer &&)> ready_function;

        http_client(http_version version, uint8_t max_sockets = 8);
        ~http_client();

        void send(request_pointer && request, ready_function on_ready, error_function on_error);
    protected:
        bool default_verify(bool preverified, boost::asio::ssl::verify_context & context);
    private:
        http_version m_version;
        std::atomic_bool m_is_running;
        uint8_t m_max_sockets;
        std::atomic<uint8_t> m_socket_count;
        std::thread m_thread;
        std::mutex m_mutex_work;
        std::queue<std::function<void()>> m_pending_work;

        boost::asio::io_service m_io_service;
        work_pointer m_infinite_work;

        std::string const to_string(http_request const & request) const;
        std::string const get_header_from_stream(boost::asio::streambuf & streambuf, size_t bytes_read) const;
        size_t const get_chunk_length(std::string const & contents) const;
        std::string const get_body_from_stream(boost::asio::streambuf & streambuf, size_t bytes_read, size_t expected_size = 0) const;
        std::string & erase_chunk_length(std::string & contents) const;
        std::string const zlib_inflate(void const * deflated_buffer, unsigned int deflated_size) const;

        void do_connect_async(request_pointer && request, ready_function on_ready, error_function on_error);

        void on_connect(context_pointer context);
        void on_handshake(context_pointer context);
        void on_read(context_pointer context, size_t bytes_read);
        void on_write(context_pointer context, size_t bytes_written);
    };
}

#endif