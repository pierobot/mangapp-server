#ifndef MANGAPP_HTTP_HELPER_HPP
#define MANGAPP_HTTP_HELPER_HPP

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#endif

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include <boost/asio.hpp>

namespace mangapp
{
    struct http_context
    {
        std::string host;
        std::string url;
        std::string contents;
        
        boost::asio::streambuf stream;

        uint32_t current_chunk;
        bool is_chunked;
        union
        {
            size_t content_length;
            size_t chunk_length;
        }; 
        size_t bytes_transferred;

        std::function<void(std::string const &)> on_ready;
        std::function<void(std::string const &)> on_error;
        
        http_context() : 
            host(), url(), contents(), stream(), current_chunk(0), is_chunked(0), content_length(0), bytes_transferred(0),
            on_ready(nullptr), on_error(nullptr)
        {
        }
    };

    class http_helper
    {
    public:
        typedef std::pair<std::string::const_iterator, std::string::const_iterator> iterator_range;
        typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_pointer;
        typedef std::shared_ptr<std::string> string_pointer;
        typedef std::shared_ptr<http_context> context_pointer;
        
        typedef std::shared_ptr<boost::asio::io_service::work> work_pointer;


        typedef std::function<void(boost::system::error_code const &, size_t)> on_io_function;
        typedef std::function<void(std::string const &)> on_ready_function;
        typedef std::function<void(std::string const &)> on_error_function;

        http_helper();
        virtual ~http_helper();

        void http_get_async(std::string const & host,
                            std::string const & url,
                            on_ready_function on_ready,
                            on_error_function on_error);
    protected:
        void on_connect(socket_pointer socket, context_pointer context);
        void on_read(socket_pointer socket, context_pointer context, size_t bytes_read);
        void on_write(socket_pointer socket, context_pointer context, size_t bytes_written);
    private:
        std::string const get_header_from_stream(boost::asio::streambuf & streambuf, size_t bytes_read) const;
        std::string const get_contents_from_stream(boost::asio::streambuf & streambuf, size_t bytes_read, size_t expected_size = 0) const;
        std::string & erase_chunk_length(std::string & contents) const;

        std::string const build_request_header(std::string const & host, std::string const & url) const;
        std::string const extract_http_content(std::string const & contents) const;
        size_t const get_header_length(std::string const & headers) const;
        size_t const get_content_length(std::string const & headers) const;
        

        bool is_chunked(std::string const & headers) const;
        size_t const get_chunk_length(std::string const & contents) const;

        std::atomic_bool m_is_running;
        std::thread m_thread;
        boost::asio::io_service m_io_service;
        work_pointer m_infinite_work;
    };
}

#endif