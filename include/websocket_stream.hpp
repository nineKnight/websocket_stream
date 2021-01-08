//
// Copyright (c) 2021 nineKnight (mikezhen0707 at gmail dot com)
//

#ifndef WEBSOCKET_STREAM_HPP
#define WEBSOCKET_STREAM_HPP

#include "websocket_stream_base.hpp"

namespace boost {
namespace beast {
namespace websocket {

template <class Derived>
class websocket_stream
    : public websocket_stream_base
#if !BOOST_BEAST_DOXYGEN
    , private stream_base
#endif
{
    // Access the derived class, this is part of
    // the Curiously Recurring Template Pattern idiom.
    Derived& derived() { return static_cast<Derived&>(*this); }

  public:
    //--------------------------------------------------------------------------

    virtual executor_type get_executor() noexcept override
    {
        return derived().ws().get_executor();
    }

    virtual lowest_layer_type& lowest_layer() noexcept override
    {
        return beast::get_lowest_layer(derived().ws());
    }

    auto& next_layer() noexcept { return derived().ws().next_layer(); }

    virtual bool is_open() noexcept override
    {
        return derived().ws().is_open();
    }

    virtual bool got_binary() noexcept override
    {
        return derived().ws().got_binary();
    }

    virtual bool got_text() noexcept override
    {
        return derived().ws().got_text();
    }

    virtual bool is_message_done() noexcept override
    {
        return derived().ws().is_message_done();
    }

    virtual close_reason& reason() noexcept override
    {
        return const_cast<close_reason&>(derived().ws().reason());
    }

    virtual std::size_t read_size_hint(
        std::size_t initial_size = +tcp_frame_size) override
    {
        return derived().ws().read_size_hint(initial_size);
    }

    template <class DynamicBuffer
#if !BOOST_BEAST_DOXYGEN
              ,
              class = typename std::enable_if<
                  !std::is_integral<DynamicBuffer>::value>::type
#endif
              >
    std::size_t read_size_hint(DynamicBuffer& buffer)
    {
        return derived().ws().read_size_hint(buffer);
    }

#if BOOST_BEAST_DOXYGEN
    template <class Option>
    void get_option(Option& opt)
    {
        return derived().ws().get_option(opt);
    }

    template <class Option>
    void set_option(Option opt)
    {
        return derived().ws().set_option(opt);
    }
#else

    void set_option(decorator opt)
    {
        return derived().ws().set_option(std::move(opt));
    }

    void get_option(timeout& opt) { return derived().ws().get_option(opt); }
    void set_option(timeout const& opt)
    {
        return derived().ws().set_option(opt);
    }
#endif

    virtual void set_option(permessage_deflate const& o) override
    {
        return derived().ws().set_option(o);
    }

    virtual void get_option(permessage_deflate& o) override
    {
        return derived().ws().get_option(o);
    }

    virtual void auto_fragment(bool value) override
    {
        return derived().ws().auto_fragment(value);
    }

    virtual bool auto_fragment() override
    {
        return derived().ws().auto_fragment();
    }

    virtual void binary(bool value) override
    {
        return derived().ws().binary(value);
    }

    virtual bool binary() override { return derived().ws().binary(); }

    virtual void control_callback(
        std::function<void(frame_type, string_view)> cb) override
    {
        return derived().ws().control_callback(cb);
    }

    virtual void control_callback() override
    {
        return derived().ws().control_callback();
    }

    virtual void read_message_max(std::size_t amount) override
    {
        return derived().ws().read_message_max(amount);
    }

    virtual std::size_t read_message_max() override
    {
        return derived().ws().read_message_max();
    }

    virtual void secure_prng(bool value) override
    {
        return derived().ws().secure_prng(value);
    }

    virtual void write_buffer_bytes(std::size_t amount) override
    {
        return derived().ws().write_buffer_bytes(amount);
    }

    virtual std::size_t write_buffer_bytes() override
    {
        return derived().ws().write_buffer_bytes();
    }

    virtual void text(bool value) override
    {
        return derived().ws().text(value);
    }

    virtual bool text() override { return derived().ws().text(); }

    virtual void handshake(string_view host, string_view target) override
    {
        return derived().ws().handshake(host, target);
    }

    virtual void handshake(response_type& res, string_view host,
                           string_view target) override
    {
        return derived().ws().handshake(res, host, target);
    }

    virtual void handshake(string_view host, string_view target,
                           error_code& ec) override
    {
        return derived().ws().handshake(host, target, ec);
    }

    virtual void handshake(response_type& res, string_view host,
                           string_view target, error_code& ec) override
    {
        return derived().ws().handshake(res, host, target, ec);
    }

    template <BOOST_BEAST_ASYNC_TPARAM1 HandshakeHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT1(HandshakeHandler)
    async_handshake(string_view host, string_view target,
                    HandshakeHandler&& handler =
                        net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_handshake(host, target, handler);
    }

    template <BOOST_BEAST_ASYNC_TPARAM1 HandshakeHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT1(HandshakeHandler)
    async_handshake(response_type& res, string_view host, string_view target,
                    HandshakeHandler&& handler =
                        net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_handshake(res, host, target, handler);
    }

    virtual void accept() override { return derived().ws().accept(); }

    virtual void accept(error_code& ec) override
    {
        return derived().ws().accept(ec);
    }

    template <class ConstBufferSequence>
#if BOOST_BEAST_DOXYGEN
    void
#else
    typename std::enable_if<
        !http::detail::is_header<ConstBufferSequence>::value>::type
#endif
    accept(ConstBufferSequence const& buffers)
    {
        return derived().ws().accept(buffers);
    }

    template <class ConstBufferSequence>
#if BOOST_BEAST_DOXYGEN
    void
#else
    typename std::enable_if<
        !http::detail::is_header<ConstBufferSequence>::value>::type
#endif
    accept(ConstBufferSequence const& buffers, error_code& ec)
    {
        return derived().ws().accept(buffers, ec);
    }

    template <class Body, class Allocator>
    void accept(http::request<Body, http::basic_fields<Allocator>> const& req)
    {
        return derived().ws().accept(req);
    }

    template <class Body, class Allocator>
    void accept(http::request<Body, http::basic_fields<Allocator>> const& req,
                error_code& ec)
    {
        return derived().ws().accept(req, ec);
    }

    template <BOOST_BEAST_ASYNC_TPARAM1 AcceptHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT1(AcceptHandler)
    async_accept(AcceptHandler&& handler =
                     net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_accept(handler);
    }

    template <class ConstBufferSequence,
              BOOST_BEAST_ASYNC_TPARAM1 AcceptHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT1(AcceptHandler)
    async_accept(
        ConstBufferSequence const& buffers,
        AcceptHandler&& handler =
            net::default_completion_token_t<executor_type> {}
#ifndef BOOST_BEAST_DOXYGEN
        ,
        typename std::enable_if<
            !http::detail::is_header<ConstBufferSequence>::value>::type* = 0
#endif
    )
    {
        return derived().ws().async_accept(buffers, handler);
    }

    template <class Body, class Allocator,
              BOOST_BEAST_ASYNC_TPARAM1 AcceptHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT1(AcceptHandler)
    async_accept(http::request<Body, http::basic_fields<Allocator>> const& req,
                 AcceptHandler&& handler =
                     net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_accept(req, handler);
    }

    virtual void close(close_reason const& cr) override
    {
        return derived().ws().close(cr);
    }

    virtual void close(close_reason const& cr, error_code& ec) override
    {
        return derived().ws().close(cr, ec);
    }

    template <BOOST_BEAST_ASYNC_TPARAM1 CloseHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT1(CloseHandler)
    async_close(close_reason const& cr,
                CloseHandler&& handler =
                    net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_close(cr, handler);
    }

    virtual void ping(ping_data const& payload) override
    {
        return derived().ws().ping(payload);
    }

    virtual void ping(ping_data const& payload, error_code& ec) override
    {
        return derived().ws().ping(payload, ec);
    }

    template <BOOST_BEAST_ASYNC_TPARAM1 WriteHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT1(WriteHandler)
    async_ping(ping_data const& payload,
               WriteHandler&& handler =
                   net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_ping(payload, handler);
    }

    virtual void pong(ping_data const& payload) override
    {
        return derived().ws().pong(payload);
    }

    virtual void pong(ping_data const& payload, error_code& ec) override
    {
        return derived().ws().pong(payload, ec);
    }

    template <BOOST_BEAST_ASYNC_TPARAM1 WriteHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT1(WriteHandler)
    async_pong(ping_data const& payload,
               WriteHandler&& handler =
                   net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_pong(payload, handler);
    }

    template <class DynamicBuffer>
    std::size_t read(DynamicBuffer& buffer)
    {
        return derived().ws().read(buffer);
    }

    template <class DynamicBuffer>
    std::size_t read(DynamicBuffer& buffer, error_code& ec)
    {
        return derived().ws().read(buffer, ec);
    }

    template <class DynamicBuffer,
              BOOST_BEAST_ASYNC_TPARAM2 ReadHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
    async_read(DynamicBuffer& buffer,
               ReadHandler&& handler =
                   net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_read(buffer, handler);
    }

    //--------------------------------------------------------------------------

    template <class DynamicBuffer>
    std::size_t read_some(DynamicBuffer& buffer, std::size_t limit)
    {
        return derived().ws().read_some(buffer, limit);
    }

    template <class DynamicBuffer>
    std::size_t read_some(DynamicBuffer& buffer, std::size_t limit,
                          error_code& ec)
    {
        return derived().ws().read_some(buffer, limit, ec);
    }

    template <class DynamicBuffer,
              BOOST_BEAST_ASYNC_TPARAM2 ReadHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
    async_read_some(DynamicBuffer& buffer, std::size_t limit,
                    ReadHandler&& handler =
                        net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_read_some(buffer, limit, handler);
    }

    //--------------------------------------------------------------------------

    template <class MutableBufferSequence>
    std::size_t read_some(MutableBufferSequence const& buffers)
    {
        return derived().ws().read_some(buffers);
    }

    template <class MutableBufferSequence>
    std::size_t read_some(MutableBufferSequence const& buffers, error_code& ec)
    {
        return derived().ws().read_some(buffers, ec);
    }

    template <class MutableBufferSequence,
              BOOST_BEAST_ASYNC_TPARAM2 ReadHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
    async_read_some(MutableBufferSequence const& buffers,
                    ReadHandler&& handler =
                        net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_read_some(buffers, handler);
    }

    template <class ConstBufferSequence>
    std::size_t write(ConstBufferSequence const& buffers)
    {
        return derived().ws().write(buffers);
    }

    template <class ConstBufferSequence>
    std::size_t write(ConstBufferSequence const& buffers, error_code& ec)
    {
        return derived().ws().write(buffers, ec);
    }

    template <class ConstBufferSequence,
              BOOST_BEAST_ASYNC_TPARAM2 WriteHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT2(WriteHandler)
    async_write(ConstBufferSequence const& buffers,
                WriteHandler&& handler =
                    net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_write(buffers, handler);
    }

    template <class ConstBufferSequence>
    std::size_t write_some(bool fin, ConstBufferSequence const& buffers)
    {
        return derived().ws().write_some(fin, buffers);
    }

    template <class ConstBufferSequence>
    std::size_t write_some(bool fin, ConstBufferSequence const& buffers,
                           error_code& ec)
    {
        return derived().ws().write_some(fin, buffers, ec);
    }

    template <class ConstBufferSequence,
              BOOST_BEAST_ASYNC_TPARAM2 WriteHandler =
                  net::default_completion_token_t<executor_type>>
    BOOST_BEAST_ASYNC_RESULT2(WriteHandler)
    async_write_some(bool fin, ConstBufferSequence const& buffers,
                     WriteHandler&& handler =
                         net::default_completion_token_t<executor_type>{})
    {
        return derived().ws().async_write_some(fin, buffers, handler);
    }
};

//------------------------------------------------------------------------------

// Handles a plain WebSocket connection
class plain_websocket_stream
    : public websocket_stream<plain_websocket_stream>
    , public std::enable_shared_from_this<plain_websocket_stream>
{
    websocket::stream<beast::tcp_stream> ws_;

  public:
    // Create the plain_websocket_stream
    // explicit plain_websocket_stream(beast::tcp_stream&& stream)
    //    : ws_(std::move(stream))
    //{
    //    use_ssl_ = false;
    //}
    template <class... Args>
    explicit plain_websocket_stream(Args&&... args)
        : ws_(std::forward<Args>(args)...)
    {
        use_ssl_ = false;
    }

    // Called by the base class
    websocket::stream<beast::tcp_stream>& ws() { return ws_; }
};

//------------------------------------------------------------------------------

// Handles an SSL WebSocket connection
class ssl_websocket_stream
    : public websocket_stream<ssl_websocket_stream>
    , public std::enable_shared_from_this<ssl_websocket_stream>
{
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;

  public:
    // Create the ssl_websocket_stream
    // explicit ssl_websocket_stream(beast::ssl_stream<beast::tcp_stream>&&
    // stream)
    //    : ws_(std::move(stream))
    //{
    //    use_ssl_ = true;
    //}
    template <class... Args>
    explicit ssl_websocket_stream(Args&&... args)
        : ws_(std::forward<Args>(args)...)
    {
        use_ssl_ = true;
    }

    // Called by the base class
    websocket::stream<beast::ssl_stream<beast::tcp_stream>>& ws()
    {
        return ws_;
    }
};

}    // namespace websocket
}    // namespace beast
}    // namespace boost

using ssl_websocket_stream = boost::beast::websocket::ssl_websocket_stream;
using plain_websocket_stream = boost::beast::websocket::plain_websocket_stream;

#endif    // !WEBSOCKET_STREAM_HPP
