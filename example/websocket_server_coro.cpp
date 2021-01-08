//
// Copyright (c) 2021 nineKnight (mikezhen0707 at gmail dot com)
//

//------------------------------------------------------------------------------
//
// Example: WebSocket server, coroutine (plain + SSL)
//
//------------------------------------------------------------------------------

#include "websocket_stream.hpp"
#include "example/common/server_certificate.hpp"

#include <algorithm>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;            // from <boost/beast.hpp>
namespace http = beast::http;              // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;    // from <boost/beast/websocket.hpp>
namespace net = boost::asio;               // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;          // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
void do_session(beast::tcp_stream& stream, net::yield_context yield)
{
    beast::error_code ec;

    stream.expires_after(std::chrono::seconds(30));
    beast::flat_buffer buffer;
    bool result = true;
    result = beast::async_detect_ssl(stream, buffer, yield[ec]);
    if (ec)
        return fail(ec, "detect_ssl");
    std::cout << "detect_ssl: " << result << std::endl;

    std::unique_ptr<websocket_stream_base> ws_ptr;
    boost::optional<http::request_parser<http::string_body>> parser;
    parser.emplace();
    parser->body_limit(10000);
    result = true;
    if (result) {
        boost::asio::ssl::context ctx(boost::asio::ssl::context_base::tlsv12);
        load_server_certificate(ctx);
        beast::ssl_stream<beast::tcp_stream> sstream(std::move(stream), ctx);
        beast::get_lowest_layer(sstream).expires_after(
            std::chrono::seconds(30));

        std::size_t bytes_used;
        bytes_used = sstream.async_handshake(
            boost::asio::ssl::stream_base::server, buffer.data(), yield[ec]);
        if (ec)
            return fail(ec, "handshake");
        buffer.consume(bytes_used);

        beast::get_lowest_layer(sstream).expires_after(
            std::chrono::seconds(30));
        http::async_read(sstream, buffer, *parser, yield[ec]);
        if (ec == http::error::end_of_stream) {
            beast::get_lowest_layer(sstream).expires_after(
                std::chrono::seconds(30));

            sstream.async_shutdown(yield[ec]);
            if (ec)
                return fail(ec, "shutdown");
            return;
        }

        if (ec)
            return fail(ec, "read");

        if (websocket::is_upgrade(parser->get())) {
            beast::get_lowest_layer(sstream).expires_never();

            ws_ptr.reset(new ssl_websocket_stream(std::move(sstream)));
        }
    } else {
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
        http::async_read(stream, buffer, *parser, yield[ec]);
        if (ec == http::error::end_of_stream) {
            stream.socket().shutdown(tcp::socket::shutdown_send, ec);
            return;
        }

        if (ec)
            return fail(ec, "read");

        if (websocket::is_upgrade(parser->get())) {
            beast::get_lowest_layer(stream).expires_never();

            ws_ptr.reset(new plain_websocket_stream(std::move(stream)));
        }
    }

    websocket_stream_base& ws = *ws_ptr;
    // Set suggested timeout settings for the websocket
    ws.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws.set_option(
        websocket::stream_base::decorator([](websocket::response_type& res) {
            res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-coro");
        }));

    // Accept the websocket handshake
    // ws.async_accept(yield[ec]);
    if (ws.use_ssl()) {
        websocket_stream_base::async_accept(
            static_cast<ssl_websocket_stream&>(ws), parser->release(),
            yield[ec]);
    } else {
        websocket_stream_base::async_accept(
            static_cast<plain_websocket_stream&>(ws), parser->release(),
            yield[ec]);
    }
    if (ec)
        return fail(ec, "accept");

    for (;;) {
        // This buffer will hold the incoming message
        beast::flat_buffer buffer;

        // Read a message
        // ws.async_read(buffer, yield[ec]);
        if (ws.use_ssl()) {
            websocket_stream_base::async_read(
                static_cast<ssl_websocket_stream&>(ws), buffer, yield[ec]);
        } else {
            websocket_stream_base::async_read(
                static_cast<plain_websocket_stream&>(ws), buffer, yield[ec]);
        }

        // This indicates that the session was closed
        if (ec == websocket::error::closed)
            break;

        if (ec)
            return fail(ec, "read");

        std::cout << "Receive " << '\"'
                  << std::string(reinterpret_cast<char*>(buffer.data().data()),
                                 buffer.size())
                  << '\"' << " from "
                  << ws.lowest_layer()
                         .socket()
                         .remote_endpoint()
                         .address()
                         .to_string()
                  << ":" << ws.lowest_layer().socket().remote_endpoint().port()
                  << std::endl;

        // Echo the message back
        ws.text(ws.got_text());
        // ws.async_write(buffer.data(), yield[ec]);
        if (ws.use_ssl()) {
            websocket_stream_base::async_write(
                static_cast<ssl_websocket_stream&>(ws), buffer.data(),
                yield[ec]);
        } else {
            websocket_stream_base::async_write(
                static_cast<plain_websocket_stream&>(ws), buffer.data(),
                yield[ec]);
        }
        if (ec)
            return fail(ec, "write");
    }
}

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
void do_listen(net::io_context& ioc, tcp::endpoint endpoint,
               net::yield_context yield)
{
    beast::error_code ec;

    // Open the acceptor
    tcp::acceptor acceptor(ioc);
    acceptor.open(endpoint.protocol(), ec);
    if (ec)
        return fail(ec, "open");

    // Allow address reuse
    acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
        return fail(ec, "set_option");

    // Bind to the server address
    acceptor.bind(endpoint, ec);
    if (ec)
        return fail(ec, "bind");

    // Start listening for connections
    acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec)
        return fail(ec, "listen");

    for (;;) {
        tcp::socket socket(ioc);
        acceptor.async_accept(socket, yield[ec]);
        if (ec)
            fail(ec, "accept");
        else
            boost::asio::spawn(
                acceptor.get_executor(),
                std::bind(&do_session, beast::tcp_stream(std::move(socket)),
                          std::placeholders::_1));
    }
}

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 4) {
        std::cerr << "Usage: websocket-server-coro <address> <port> <threads>\n"
                  << "Example:\n"
                  << "    websocket-server-coro 0.0.0.0 8080 1\n";
        return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    // The io_context is required for all I/O
    net::io_context ioc(threads);

    // Spawn a listening port
    boost::asio::spawn(
        ioc, std::bind(&do_listen, std::ref(ioc), tcp::endpoint{address, port},
                       std::placeholders::_1));

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] { ioc.run(); });
    ioc.run();

    return EXIT_SUCCESS;
}
