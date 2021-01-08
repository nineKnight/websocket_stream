//
// Copyright (c) 2021 nineKnight (mikezhen0707 at gmail dot com)
//

#ifndef WEBSOCKET_STREAM_BASE_HPP
#define WEBSOCKET_STREAM_BASE_HPP

#if 1
#ifndef BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT
#define BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT
#endif
#include <boost/asio/executor.hpp>
#else
#include <boost/asio/any_io_executor.hpp>
#endif
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/option.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/websocket/stream_base.hpp>

namespace boost {
namespace beast {
namespace websocket {

class websocket_stream_base
{
    static std::size_t constexpr tcp_frame_size = 1536;

  protected:
    bool use_ssl_;

  public:
    using lowest_layer_type = tcp_stream;

#if 1
    using executor_type = net::executor;
#else
    using executor_type = net::any_io_executor;
#endif

    websocket_stream_base() : use_ssl_(false) {}

    virtual ~websocket_stream_base() {}

    virtual bool use_ssl() noexcept { return use_ssl_; }

    //--------------------------------------------------------------------------

    /** Get the executor associated with the object.

        This function may be used to obtain the executor object that the
        stream uses to dispatch handlers for asynchronous operations.

        @return A copy of the executor that stream will use to dispatch
       handlers.
    */
    virtual executor_type get_executor() = 0;

    /** Get a reference to the next layer

        This function returns a reference to the next layer
        in a stack of stream layers.

        @return A reference to the next layer in the stack of
        stream layers.
    */
    virtual lowest_layer_type& lowest_layer() = 0;

    //--------------------------------------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------------------------------------

    /** Returns `true` if the stream is open.

        The stream is open after a successful handshake, and when
        no error has occurred.
    */
    virtual bool is_open() = 0;

    /** Returns `true` if the latest message data indicates binary.

        This function informs the caller of whether the last
        received message frame represents a message with the
        binary opcode.

        If there is no last message frame, the return value is
        undefined.
    */
    virtual bool got_binary() = 0;

    /** Returns `true` if the latest message data indicates text.

        This function informs the caller of whether the last
        received message frame represents a message with the
        text opcode.

        If there is no last message frame, the return value is
        undefined.
    */
    virtual bool got_text() = 0;

    /// Returns `true` if the last completed read finished the current message.
    virtual bool is_message_done() = 0;

    /** Returns the close reason received from the remote peer.

        This is only valid after a read completes with error::closed.
    */
    virtual close_reason& reason() = 0;

    /** Returns a suggested maximum buffer size for the next call to read.

        This function returns a reasonable upper limit on the number
        of bytes for the size of the buffer passed in the next call
        to read. The number is determined by the state of the current
        frame and whether or not the permessage-deflate extension is
        enabled.

        @param initial_size A non-zero size representing the caller's
        desired buffer size for when there is no information which may
        be used to calculate a more specific value. For example, when
        reading the first frame header of a message.
    */
    virtual std::size_t read_size_hint(
        std::size_t initial_size = +tcp_frame_size) = 0;

    /** Returns a suggested maximum buffer size for the next call to read.

        This function returns a reasonable upper limit on the number
        of bytes for the size of the buffer passed in the next call
        to read. The number is determined by the state of the current
        frame and whether or not the permessage-deflate extension is
        enabled.

        @param buffer The buffer which will be used for reading. The
        implementation will query the buffer to obtain the optimum
        size of a subsequent call to `buffer.prepare` based on the
        state of the current frame, if any.
    */
    template <class WebsocketStream, class DynamicBuffer
#if !BOOST_BEAST_DOXYGEN
              ,
              class = typename std::enable_if<
                  !std::is_integral<DynamicBuffer>::value>::type
#endif
              >
    static std::size_t read_size_hint(WebsocketStream& stream,
                                      DynamicBuffer& buffer)
    {
        return stream.ws().read_size_hint(buffer);
    }

    //--------------------------------------------------------------------------
    //
    // Settings
    //
    //--------------------------------------------------------------------------

#if BOOST_BEAST_DOXYGEN
    template <class WebsocketStream, class Option>
    static void get_option(WebsocketStream& stream, Option& opt)
    {
        return stream.ws().get_option(opt);
    }

    template <class WebsocketStream, class Option>
    static void set_option(WebsocketStream& stream, Option opt)
    {
        return stream.ws().set_option(opt);
    }
#else

    virtual void set_option(stream_base::decorator opt) = 0;

    virtual void get_option(stream_base::timeout& opt) = 0;
    virtual void set_option(stream_base::timeout const& opt) = 0;
#endif

    /** Set the permessage-deflate extension options

        @throws invalid_argument if `deflateSupported == false`, and either
        `client_enable` or `server_enable` is `true`.
    */
    virtual void set_option(permessage_deflate const& o) = 0;

    /// Get the permessage-deflate extension options
    virtual void get_option(permessage_deflate& o) = 0;

    /** Set the automatic fragmentation option.

        Determines if outgoing message payloads are broken up into
        multiple pieces.

        When the automatic fragmentation size is turned on, outgoing
        message payloads are broken up into multiple frames no larger
        than the write buffer size.

        The default setting is to fragment messages.

        @param value A `bool` indicating if auto fragmentation should be on.

        @par Example
        Setting the automatic fragmentation option:
        @code
            ws.auto_fragment(true);
        @endcode
    */
    virtual void auto_fragment(bool value) = 0;

    /// Returns `true` if the automatic fragmentation option is set.
    virtual bool auto_fragment() = 0;

    /** Set the binary message write option.

        This controls whether or not outgoing message opcodes
        are set to binary or text. The setting is only applied
        at the start when a caller begins a new message. Changing
        the opcode after a message is started will only take effect
        after the current message being sent is complete.

        The default setting is to send text messages.

        @param value `true` if outgoing messages should indicate
        binary, or `false` if they should indicate text.

        @par Example
        Setting the message type to binary.
        @code
            ws.binary(true);
        @endcode
        */
    virtual void binary(bool value) = 0;

    /// Returns `true` if the binary message write option is set.
    virtual bool binary() = 0;

    /** Set a callback to be invoked on each incoming control frame.

        Sets the callback to be invoked whenever a ping, pong,
        or close control frame is received during a call to one
        of the following functions:

        @li @ref beast::websocket::stream::read
        @li @ref beast::websocket::stream::read_some
        @li @ref beast::websocket::stream::async_read
        @li @ref beast::websocket::stream::async_read_some

        Unlike completion handlers, the callback will be invoked
        for each control frame during a call to any synchronous
        or asynchronous read function. The operation is passive,
        with no associated error code, and triggered by reads.

        For close frames, the close reason code may be obtained by
        calling the function @ref reason.

        @param cb The function object to call, which must be
        invocable with this equivalent signature:
        @code
        void
        callback(
            frame_type kind,       // The type of frame
            string_view payload    // The payload in the frame
        );
        @endcode
        The implementation type-erases the callback which may require
        a dynamic allocation. To prevent the possibility of a dynamic
        allocation, use `std::ref` to wrap the callback.
        If the read operation which receives the control frame is
        an asynchronous operation, the callback will be invoked using
        the same method as that used to invoke the final handler.

        @note Incoming ping and close frames are automatically
        handled. Pings are responded to with pongs, and a close frame
        is responded to with a close frame leading to the closure of
        the stream. It is not necessary to manually send pings, pongs,
        or close frames from inside the control callback.
        Attempting to manually send a close frame from inside the
        control callback after receiving a close frame will result
        in undefined behavior.
    */
    virtual void control_callback(
        std::function<void(frame_type, string_view)> cb) = 0;

    /** Reset the control frame callback.

        This function removes any previously set control frame callback.
    */
    virtual void control_callback() = 0;

    /** Set the maximum incoming message size option.

        Sets the largest permissible incoming message size. Message
        frame fields indicating a size that would bring the total
        message size over this limit will cause a protocol failure.

        The default setting is 16 megabytes. A value of zero indicates
        a limit of the maximum value of a `std::uint64_t`.

        @par Example
        Setting the maximum read message size.
        @code
            ws.read_message_max(65536);
        @endcode

        @param amount The limit on the size of incoming messages.
    */
    virtual void read_message_max(std::size_t amount) = 0;

    /// Returns the maximum incoming message size setting.
    virtual std::size_t read_message_max() = 0;

    /** Set whether the PRNG is cryptographically secure

        This controls whether or not the source of pseudo-random
        numbers used to produce the masks required by the WebSocket
        protocol are of cryptographic quality. When the setting is
        `true`, a strong algorithm is used which cannot be guessed
        by observing outputs. When the setting is `false`, a much
        faster algorithm is used.
        Masking is only performed by streams operating in the client
        mode. For streams operating in the server mode, this setting
        has no effect.
        By default, newly constructed streams use a secure PRNG.

        If the WebSocket stream is used with an encrypted SSL or TLS
        next layer, if it is known to the application that intermediate
        proxies are not vulnerable to cache poisoning, or if the
        application is designed such that an attacker cannot send
        arbitrary inputs to the stream interface, then the faster
        algorithm may be used.

        For more information please consult the WebSocket protocol RFC.

        @param value `true` if the PRNG algorithm should be
        cryptographically secure.
    */
    virtual void secure_prng(bool value) = 0;

    /** Set the write buffer size option.

        Sets the size of the write buffer used by the implementation to
        send frames. The write buffer is needed when masking payload data
        in the client role, compressing frames, or auto-fragmenting message
        data.

        Lowering the size of the buffer can decrease the memory requirements
        for each connection, while increasing the size of the buffer can reduce
        the number of calls made to the next layer to write data.

        The default setting is 4096. The minimum value is 8.

        The write buffer size can only be changed when the stream is not
        open. Undefined behavior results if the option is modified after a
        successful WebSocket handshake.

        @par Example
        Setting the write buffer size.
        @code
            ws.write_buffer_bytes(8192);
        @endcode

        @param amount The size of the write buffer in bytes.
    */
    virtual void write_buffer_bytes(std::size_t amount) = 0;

    /// Returns the size of the write buffer.
    virtual std::size_t write_buffer_bytes() = 0;

    /** Set the text message write option.

        This controls whether or not outgoing message opcodes
        are set to binary or text. The setting is only applied
        at the start when a caller begins a new message. Changing
        the opcode after a message is started will only take effect
        after the current message being sent is complete.

        The default setting is to send text messages.

        @param value `true` if outgoing messages should indicate
        text, or `false` if they should indicate binary.

        @par Example
        Setting the message type to text.
        @code
            ws.text(true);
        @endcode
    */
    virtual void text(bool value) = 0;

    /// Returns `true` if the text message write option is set.
    virtual bool text() = 0;

    /*
        timer settings

        * Timer is disabled
        * Close on timeout
            - no complete frame received, OR
            - no complete frame sent
        * Ping on timeout
            - ping on no complete frame received
                * if can't ping?
    */

    //--------------------------------------------------------------------------
    //
    // Handshaking (Client)
    //
    //--------------------------------------------------------------------------

    /** Perform the WebSocket handshake in the client role.

        This function is used to perform the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        The call blocks until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.

        @throws system_error Thrown on failure.

        @par Example
        @code
        ws.handshake("localhost", "/");
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket
       Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form
       (RFC7230)</a>
    */
    virtual void handshake(string_view host, string_view target) = 0;

    /** Perform the WebSocket handshake in the client role.

        This function is used to perform the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        The call blocks until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols.

        @param res The HTTP Upgrade response returned by the remote
        endpoint. The caller may use the response to access any
        additional information sent by the server.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.

        @throws system_error Thrown on failure.

        @par Example
        @code
        response_type res;
        ws.handshake(res, "localhost", "/");
        std::cout << res;
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket
       Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form
       (RFC7230)</a>
    */
    virtual void handshake(response_type& res, string_view host,
                           string_view target) = 0;

    /** Perform the WebSocket handshake in the client role.

        This function is used to perform the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        The call blocks until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.

        @param ec Set to indicate what error occurred, if any.

        @par Example
        @code
        error_code ec;
        ws.handshake("localhost", "/", ec);
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket
       Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form
       (RFC7230)</a>
    */
    virtual void handshake(string_view host, string_view target,
                           error_code& ec) = 0;

    /** Perform the WebSocket handshake in the client role.

        This function is used to perform the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        The call blocks until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols.

        @param res The HTTP Upgrade response returned by the remote
        endpoint. The caller may use the response to access any
        additional information sent by the server.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.

        @param ec Set to indicate what error occurred, if any.

        @par Example
        @code
        error_code ec;
        response_type res;
        ws.handshake(res, "localhost", "/", ec);
        if(! ec)
            std::cout << res;
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket
       Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form
       (RFC7230)</a>
    */
    virtual void handshake(response_type& res, string_view host,
                           string_view target, error_code& ec) = 0;

    /** Perform the WebSocket handshake asynchronously in the client role.

        This initiating function is used to asynchronously begin performing the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. No other operation may be performed
        on the stream until this operation completes.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.
        The implementation will not access the string data after the
        initiating function returns.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.
        The implementation will not access the string data after the
        initiating function returns.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec    // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @par Example
        @code
        ws.async_handshake("localhost", "/",
            [](error_code ec)
            {
                if(ec)
                    std::cerr << "Error: " << ec.message() << "\n";
            });
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket
       Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form
       (RFC7230)</a>
    */
    template <class WebsocketStream,
              BOOST_BEAST_ASYNC_TPARAM1 HandshakeHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT1(HandshakeHandler)
        async_handshake(WebsocketStream& stream, string_view host,
                        string_view target,
                        HandshakeHandler&& handler =
                            net::default_completion_token_t<executor_type>{})

    {
        return stream.ws().async_handshake(host, target, handler);
    }

    /** Perform the WebSocket handshake asynchronously in the client role.

        This initiating function is used to asynchronously begin performing the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. No other operation may be performed
        on the stream until this operation completes.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols.

        @param res The HTTP Upgrade response returned by the remote
        endpoint. The caller may use the response to access any
        additional information sent by the server. This object will
        be assigned before the completion handler is invoked.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.
        The implementation will not access the string data after the
        initiating function returns.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.
        The implementation will not access the string data after the
        initiating function returns.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec    // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @par Example
        @code
        response_type res;
        ws.async_handshake(res, "localhost", "/",
            [&res](error_code ec)
            {
                if(ec)
                    std::cerr << "Error: " << ec.message() << "\n";
                else
                    std::cout << res;

            });
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket
       Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target
       (RFC7230)</a>
        @li <a
       href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form
       (RFC7230)</a>
    */
    template <class WebsocketStream,
              BOOST_BEAST_ASYNC_TPARAM1 HandshakeHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT1(HandshakeHandler)
        async_handshake(WebsocketStream& stream, response_type& res,
                        string_view host, string_view target,
                        HandshakeHandler&& handler =
                            net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_handshake(res, host, target, handler);
    }

    //--------------------------------------------------------------------------
    //
    // Handshaking (Server)
    //
    //--------------------------------------------------------------------------

    /** Perform the WebSocket handshake in the server role.

        This function is used to perform the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        The call blocks until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols is sent to the peer,
       otherwise a non-successful error is associated with the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @throws system_error Thrown on failure.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket
       Opening Handshake Server Requirements (RFC6455)</a>
    */
    virtual void accept() = 0;

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to perform the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        The call blocks until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols is sent to the peer,
       otherwise a non-successful error is associated with the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @param ec Set to indicate what error occurred, if any.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket
       Opening Handshake Server Requirements (RFC6455)</a>
    */
    virtual void accept(error_code& ec) = 0;

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to perform the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        The call blocks until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols is sent to the peer,
       otherwise a non-successful error is associated with the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @param buffers Caller provided data that has already been
        received on the stream. The implementation will copy the
        caller provided data before the function returns.

        @throws system_error Thrown on failure.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket
       Opening Handshake Server Requirements (RFC6455)</a>
    */
    template <class WebsocketStream, class ConstBufferSequence>
    static
#if BOOST_BEAST_DOXYGEN
        void
#else
        typename std::enable_if<
            !http::detail::is_header<ConstBufferSequence>::value>::type
#endif
        accept(WebsocketStream& stream, ConstBufferSequence const& buffers)
    {
        return stream.ws().accept(buffers);
    }

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to perform the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        The call blocks until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols is sent to the peer,
       otherwise a non-successful error is associated with the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @param buffers Caller provided data that has already been
        received on the stream. The implementation will copy the
        caller provided data before the function returns.

        @param ec Set to indicate what error occurred, if any.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket
       Opening Handshake Server Requirements (RFC6455)</a>
    */
    template <class WebsocketStream, class ConstBufferSequence>
    static
#if BOOST_BEAST_DOXYGEN
        void
#else
        typename std::enable_if<
            !http::detail::is_header<ConstBufferSequence>::value>::type
#endif
        accept(WebsocketStream& stream, ConstBufferSequence const& buffers,
               error_code& ec)
    {
        return stream.ws().accept(buffers, ec);
    }

    /** Respond to a WebSocket HTTP Upgrade request

        This function is used to perform the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        The call blocks until one of the following conditions is true:

        @li The response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols is sent to the peer,
       otherwise a non-successful error is associated with the operation.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not
        access this object from other threads.

        @throws system_error Thrown on failure.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket
       Opening Handshake Server Requirements (RFC6455)</a>
    */
    template <class WebsocketStream, class Body, class Allocator>
    static void accept(
        WebsocketStream& stream,
        http::request<Body, http::basic_fields<Allocator>> const& req)
    {
        return stream.ws().accept(req);
    }

    /** Respond to a WebSocket HTTP Upgrade request

        This function is used to perform the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        The call blocks until one of the following conditions is true:

        @li The response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols is sent to the peer,
       otherwise a non-successful error is associated with the operation.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not
        access this object from other threads.

        @param ec Set to indicate what error occurred, if any.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket
       Opening Handshake Server Requirements (RFC6455)</a>
    */
    template <class WebsocketStream, class Body, class Allocator>
    static void accept(
        WebsocketStream& stream,
        http::request<Body, http::basic_fields<Allocator>> const& req,
        error_code& ec)
    {
        return stream.ws().accept(req, ec);
    }

    /** Perform the WebSocket handshake asynchronously in the server role.

        This initiating function is used to asynchronously begin performing the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. No other operation may be performed
        on the stream until this operation completes.

        If a valid upgrade request is received, an HTTP response with a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols is sent to the peer,
       otherwise a non-successful error is associated with the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::async_read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec    // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket
       Opening Handshake Server Requirements (RFC6455)</a>
    */
    template <class WebsocketStream,
              BOOST_BEAST_ASYNC_TPARAM1 AcceptHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT1(AcceptHandler)
        async_accept(WebsocketStream& stream,
                     AcceptHandler&& handler =
                         net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_accept(handler);
    }

    /** Perform the WebSocket handshake asynchronously in the server role.

        This initiating function is used to asynchronously begin performing the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. No other operation may be performed
        on the stream until this operation completes.

        If a valid upgrade request is received, an HTTP response with a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols is sent to the peer,
       otherwise a non-successful error is associated with the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::async_read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @param buffers Caller provided data that has already been
        received on the stream. This may be used for implementations
        allowing multiple protocols on the same stream. The
        buffered data will first be applied to the handshake, and
        then to received WebSocket frames. The implementation will
        copy the caller provided data before the function returns.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec    // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket
       Opening Handshake Server Requirements (RFC6455)</a>
    */
    template <class WebsocketStream, class ConstBufferSequence,
              BOOST_BEAST_ASYNC_TPARAM1 AcceptHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT1(AcceptHandler) async_accept(
        WebsocketStream& stream, ConstBufferSequence const& buffers,
        AcceptHandler&& handler =
            net::default_completion_token_t<executor_type> {}
#ifndef BOOST_BEAST_DOXYGEN
        ,
        typename std::enable_if<
            !http::detail::is_header<ConstBufferSequence>::value>::type* = 0
#endif
    )
    {
        return stream.ws().async_accept(buffers, handler);
    }

    /** Perform the WebSocket handshake asynchronously in the server role.

        This initiating function is used to asynchronously begin performing the
        <a
       href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket
       handshake</a>, required before messages can be sent and received. During
       the handshake, the client sends the Websocket Upgrade HTTP request, and
       the server replies with an HTTP response indicating the result of the
       handshake.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. No other operation may be performed
        on the stream until this operation completes.

        If a valid upgrade request is received, an HTTP response with a
        <a
       href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
       of
       @ref beast::http::status::switching_protocols is sent to the peer,
       otherwise a non-successful error is associated with the operation.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not access
        this object from other threads.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec    // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket
       Opening Handshake Server Requirements (RFC6455)</a>
    */
    template <class WebsocketStream, class Body, class Allocator,
              BOOST_BEAST_ASYNC_TPARAM1 AcceptHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT1(AcceptHandler) async_accept(
        WebsocketStream& stream,
        http::request<Body, http::basic_fields<Allocator>> const& req,
        AcceptHandler&& handler =
            net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_accept(req, handler);
    }

    //--------------------------------------------------------------------------
    //
    // Close Frames
    //
    //--------------------------------------------------------------------------

    /** Send a websocket close control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">close
       frame</a>, which begins the websocket closing handshake. The session ends
       when both ends of the connection have sent and received a close frame.

        The call blocks until one of the following conditions is true:

        @li The close frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        After beginning the closing handshake, the program should not write
        further message data, pings, or pongs. Instead, the program should
        continue reading message data until an error occurs. A read returning
        @ref error::closed indicates a successful connection closure.

        @param cr The reason for the close.
        If the close reason specifies a close code other than
        @ref beast::websocket::close_code::none, the close frame is
        sent with the close code and optional reason string. Otherwise,
        the close frame is sent with no payload.

        @throws system_error Thrown on failure.

        @see
        @li <a
       href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket
       Closing Handshake (RFC6455)</a>
    */
    virtual void close(close_reason const& cr) = 0;

    /** Send a websocket close control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">close
       frame</a>, which begins the websocket closing handshake. The session ends
       when both ends of the connection have sent and received a close frame.

        The call blocks until one of the following conditions is true:

        @li The close frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        After beginning the closing handshake, the program should not write
        further message data, pings, or pongs. Instead, the program should
        continue reading message data until an error occurs. A read returning
        @ref error::closed indicates a successful connection closure.

        @param cr The reason for the close.
        If the close reason specifies a close code other than
        @ref beast::websocket::close_code::none, the close frame is
        sent with the close code and optional reason string. Otherwise,
        the close frame is sent with no payload.

        @param ec Set to indicate what error occurred, if any.

        @see
        @li <a
       href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket
       Closing Handshake (RFC6455)</a>
    */
    virtual void close(close_reason const& cr, error_code& ec) = 0;

    /** Send a websocket close control frame asynchronously.

        This function is used to asynchronously send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">close
       frame</a>, which begins the websocket closing handshake. The session ends
       when both ends of the connection have sent and received a close frame.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The close frame finishes sending.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_write_some`
        function. No other operations except for message reading operations
        should be initiated on the stream after a close operation is started.

        After beginning the closing handshake, the program should not write
        further message data, pings, or pongs. Instead, the program should
        continue reading message data until an error occurs. A read returning
        @ref error::closed indicates a successful connection closure.

        @param cr The reason for the close.
        If the close reason specifies a close code other than
        @ref beast::websocket::close_code::none, the close frame is
        sent with the close code and optional reason string. Otherwise,
        the close frame is sent with no payload.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec     // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @see
        @li <a
       href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket
       Closing Handshake (RFC6455)</a>
    */
    template <class WebsocketStream,
              BOOST_BEAST_ASYNC_TPARAM1 CloseHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT1(CloseHandler)
        async_close(WebsocketStream& stream, close_reason const& cr,
                    CloseHandler&& handler =
                        net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_close(cr, handler);
    }

    //--------------------------------------------------------------------------
    //
    // Ping/Pong Frames
    //
    //--------------------------------------------------------------------------

    /** Send a websocket ping control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">ping
       frame</a>, which usually elicits an automatic pong control frame response
       from the peer.

        The call blocks until one of the following conditions is true:

        @li The ping frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        @param payload The payload of the ping message, which may be empty.

        @throws system_error Thrown on failure.
    */
    virtual void ping(ping_data const& payload) = 0;

    /** Send a websocket ping control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">ping
       frame</a>, which usually elicits an automatic pong control frame response
       from the peer.

        The call blocks until one of the following conditions is true:

        @li The ping frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        @param payload The payload of the ping message, which may be empty.

        @param ec Set to indicate what error occurred, if any.
    */
    virtual void ping(ping_data const& payload, error_code& ec) = 0;

    /** Send a websocket ping control frame asynchronously.

        This function is used to asynchronously send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">ping
       frame</a>, which usually elicits an automatic pong control frame response
       from the peer.

        @li The ping frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_write_some`
        function. The program must ensure that no other calls to @ref ping,
        @ref pong, @ref async_ping, or @ref async_pong are performed until
        this operation completes.

        If a close frame is sent or received before the ping frame is
        sent, the error received by this completion handler will be
        `net::error::operation_aborted`.

        @param payload The payload of the ping message, which may be empty.
        The implementation will not access the contents of this object after
        the initiating function returns.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec     // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template <class WebsocketStream,
              BOOST_BEAST_ASYNC_TPARAM1 WriteHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT1(WriteHandler)
        async_ping(WebsocketStream& stream, ping_data const& payload,
                   WriteHandler&& handler =
                       net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_ping(payload, handler);
    }

    /** Send a websocket pong control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">pong
       frame</a>, which is usually sent automatically in response to a ping
       frame from the remote peer.

        The call blocks until one of the following conditions is true:

        @li The pong frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        WebSocket allows pong frames to be sent at any time, without first
        receiving a ping. An unsolicited pong sent in this fashion may
        indicate to the remote peer that the connection is still active.

        @param payload The payload of the pong message, which may be empty.

        @throws system_error Thrown on failure.
    */
    virtual void pong(ping_data const& payload) = 0;

    /** Send a websocket pong control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">pong
       frame</a>, which is usually sent automatically in response to a ping
       frame from the remote peer.

        The call blocks until one of the following conditions is true:

        @li The pong frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        WebSocket allows pong frames to be sent at any time, without first
        receiving a ping. An unsolicited pong sent in this fashion may
        indicate to the remote peer that the connection is still active.

        @param payload The payload of the pong message, which may be empty.

        @param ec Set to indicate what error occurred, if any.
    */
    virtual void pong(ping_data const& payload, error_code& ec) = 0;

    /** Send a websocket pong control frame asynchronously.

        This function is used to asynchronously send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">pong
       frame</a>, which is usually sent automatically in response to a ping
       frame from the remote peer.

        @li The pong frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_write_some`
        function. The program must ensure that no other calls to @ref ping,
        @ref pong, @ref async_ping, or @ref async_pong are performed until
        this operation completes.

        If a close frame is sent or received before the pong frame is
        sent, the error received by this completion handler will be
        `net::error::operation_aborted`.

        WebSocket allows pong frames to be sent at any time, without first
        receiving a ping. An unsolicited pong sent in this fashion may
        indicate to the remote peer that the connection is still active.

        @param payload The payload of the pong message, which may be empty.
        The implementation will not access the contents of this object after
        the initiating function returns.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec     // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template <class WebsocketStream,
              BOOST_BEAST_ASYNC_TPARAM1 WriteHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT1(WriteHandler)
        async_pong(WebsocketStream& stream, ping_data const& payload,
                   WriteHandler&& handler =
                       net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_pong(payload, handler);
    }

    //--------------------------------------------------------------------------
    //
    // Reading
    //
    //--------------------------------------------------------------------------

    /** Read a complete message.

        This function is used to read a complete message.

        The call blocks until one of the following is true:

        @li A complete message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to append message data to.

        @throws system_error Thrown on failure.
    */
    template <class WebsocketStream, class DynamicBuffer>
    static std::size_t read(WebsocketStream& stream, DynamicBuffer& buffer)
    {
        return stream.ws().read(buffer);
    }

    /** Read a complete message.

        This function is used to read a complete message.

        The call blocks until one of the following is true:

        @li A complete message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to append message data to.

        @param ec Set to indicate what error occurred, if any.
    */
    template <class WebsocketStream, class DynamicBuffer>
    static std::size_t read(WebsocketStream& stream, DynamicBuffer& buffer,
                            error_code& ec)
    {
        return stream.ws().read(buffer, ec);
    }

    /** Read a complete message asynchronously.

        This function is used to asynchronously read a complete message.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li A complete message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. The program must ensure that no other
        calls to @ref read, @ref read_some, @ref async_read, or @ref
       async_read_some are performed until this operation completes.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        Until the operation completes, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        Pong frames and close frames sent by the implementation while the
        read operation is outstanding do not prevent the application from
        also writing message data, sending pings, sending pongs, or sending
        close frames.

        @param buffer A dynamic buffer to append message data to.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec,       // Result of operation
            std::size_t bytes_written   // Number of bytes appended to buffer
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template <class WebsocketStream, class DynamicBuffer,
              BOOST_BEAST_ASYNC_TPARAM2 ReadHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
        async_read(WebsocketStream& stream, DynamicBuffer& buffer,
                   ReadHandler&& handler =
                       net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_read(buffer, handler);
    }

    //--------------------------------------------------------------------------

    /** Read some message data.

        This function is used to read some message data.

        The call blocks until one of the following is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to append message data to.

        @param limit An upper limit on the number of bytes this function
        will append into the buffer. If this value is zero, then a reasonable
        size will be chosen automatically.

        @throws system_error Thrown on failure.
    */
    template <class WebsocketStream, class DynamicBuffer>
    static std::size_t read_some(WebsocketStream& stream, DynamicBuffer& buffer,
                                 std::size_t limit)
    {
        return stream.ws().read_some(buffer, limit);
    }

    /** Read some message data.

        This function is used to read some message data.

        The call blocks until one of the following is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to append message data to.

        @param limit An upper limit on the number of bytes this function
        will append into the buffer. If this value is zero, then a reasonable
        size will be chosen automatically.

        @param ec Set to indicate what error occurred, if any.
    */
    template <class WebsocketStream, class DynamicBuffer>
    static std::size_t read_some(WebsocketStream& stream, DynamicBuffer& buffer,
                                 std::size_t limit, error_code& ec)
    {
        return stream.ws().read_some(buffer, limit, ec);
    }

    /** Read some message data asynchronously.

        This function is used to asynchronously read some message data.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. The program must ensure that no other
        calls to @ref read, @ref read_some, @ref async_read, or @ref
       async_read_some are performed until this operation completes.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        Until the operation completes, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        Pong frames and close frames sent by the implementation while the
        read operation is outstanding do not prevent the application from
        also writing message data, sending pings, sending pongs, or sending
        close frames.

        @param buffer A dynamic buffer to append message data to.

        @param limit An upper limit on the number of bytes this function
        will append into the buffer. If this value is zero, then a reasonable
        size will be chosen automatically.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec,       // Result of operation
            std::size_t bytes_written   // Number of bytes appended to buffer
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template <class WebsocketStream, class DynamicBuffer,
              BOOST_BEAST_ASYNC_TPARAM2 ReadHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
        async_read_some(WebsocketStream& stream, DynamicBuffer& buffer,
                        std::size_t limit,
                        ReadHandler&& handler =
                            net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_read_some(buffer, limit, handler);
    }

    //--------------------------------------------------------------------------

    /** Read some message data.

        This function is used to read some message data.

        The call blocks until one of the following is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffers A buffer sequence to write message data into.
        The previous contents of the buffers will be overwritten, starting
        from the beginning.

        @throws system_error Thrown on failure.
    */
    template <class WebsocketStream, class MutableBufferSequence>
    static std::size_t read_some(WebsocketStream& stream,
                                 MutableBufferSequence const& buffers)
    {
        return stream.ws().read_some(buffers);
    }

    /** Read some message data.

        This function is used to read some message data.

        The call blocks until one of the following is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffers A buffer sequence to write message data into.
        The previous contents of the buffers will be overwritten, starting
        from the beginning.

        @param ec Set to indicate what error occurred, if any.
    */
    template <class WebsocketStream, class MutableBufferSequence>
    static std::size_t read_some(WebsocketStream& stream,
                                 MutableBufferSequence const& buffers,
                                 error_code& ec)
    {
        return stream.ws().read_some(buffers, ec);
    }

    /** Read some message data asynchronously.

        This function is used to asynchronously read some message data.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. The program must ensure that no other
        calls to @ref read, @ref read_some, @ref async_read, or @ref
       async_read_some are performed until this operation completes.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        Until the operation completes, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        Pong frames and close frames sent by the implementation while the
        read operation is outstanding do not prevent the application from
        also writing message data, sending pings, sending pongs, or sending
        close frames.

        @param buffers A buffer sequence to write message data into.
        The previous contents of the buffers will be overwritten, starting
        from the beginning.
        The implementation will make copies of this object as needed, but
        but ownership of the underlying memory is not transferred. The
        caller is responsible for ensuring that the memory locations
        pointed to by the buffer sequence remain valid until the
        completion handler is called.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec,       // Result of operation
            std::size_t bytes_written   // Number of bytes written to the
       buffers
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template <class WebsocketStream, class MutableBufferSequence,
              BOOST_BEAST_ASYNC_TPARAM2 ReadHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
        async_read_some(WebsocketStream& stream,
                        MutableBufferSequence const& buffers,
                        ReadHandler&& handler =
                            net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_read_some(buffers, handler);
    }

    //--------------------------------------------------------------------------
    //
    // Writing
    //
    //--------------------------------------------------------------------------

    /** Write a complete message.

        This function is used to write a complete message.

        The call blocks until one of the following is true:

        @li The message is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        The current setting of the @ref binary option controls
        whether the message opcode is set to text or binary. If the
        @ref auto_fragment option is set, the message will be split
        into one or more frames as necessary. The actual payload contents
        sent may be transformed as per the WebSocket protocol settings.

        @param buffers The buffers containing the message to send.

        @return The number of bytes sent from the buffers.

        @throws system_error Thrown on failure.
    */
    template <class WebsocketStream, class ConstBufferSequence>
    static std::size_t write(WebsocketStream& stream,
                             ConstBufferSequence const& buffers)
    {
        return stream.ws().write(buffers);
    }

    /** Write a complete message.

        This function is used to write a complete message.

        The call blocks until one of the following is true:

        @li The complete message is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        The current setting of the @ref binary option controls
        whether the message opcode is set to text or binary. If the
        @ref auto_fragment option is set, the message will be split
        into one or more frames as necessary. The actual payload contents
        sent may be transformed as per the WebSocket protocol settings.

        @param buffers The buffers containing the message to send.

        @param ec Set to indicate what error occurred, if any.

        @return The number of bytes sent from the buffers.
    */
    template <class WebsocketStream, class ConstBufferSequence>
    static std::size_t write(WebsocketStream& stream,
                             ConstBufferSequence const& buffers, error_code& ec)
    {
        return stream.ws().write(buffers, ec);
    }

    /** Write a complete message asynchronously.

        This function is used to asynchronously write a complete message.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The complete message is written.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's
        `async_write_some` function. The program must ensure that no other
        calls to @ref write, @ref write_some, @ref async_write, or
        @ref async_write_some are performed until this operation completes.

        The current setting of the @ref binary option controls
        whether the message opcode is set to text or binary. If the
        @ref auto_fragment option is set, the message will be split
        into one or more frames as necessary. The actual payload contents
        sent may be transformed as per the WebSocket protocol settings.

        @param buffers A buffer sequence containing the entire message
        payload. The implementation will make copies of this object
        as needed, but ownership of the underlying memory is not
        transferred. The caller is responsible for ensuring that
        the memory locations pointed to by buffers remains valid
        until the completion handler is called.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec,           // Result of operation
            std::size_t bytes_transferred   // Number of bytes sent from the
                                            // buffers. If an error occurred,
                                            // this will be less than the
       buffer_size.
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template <class WebsocketStream, class ConstBufferSequence,
              BOOST_BEAST_ASYNC_TPARAM2 WriteHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT2(WriteHandler)
        async_write(WebsocketStream& stream, ConstBufferSequence const& buffers,
                    WriteHandler&& handler =
                        net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_write(buffers, handler);
    }

    /** Write some message data.

        This function is used to send part of a message.

        The call blocks until one of the following is true:

        @li The message data is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        If this is the beginning of a new message, the message opcode
        will be set to text or binary based on the current setting of
        the @ref binary (or @ref text) option. The actual payload sent
        may be transformed as per the WebSocket protocol settings.

        @param fin `true` if this is the last part of the message.

        @param buffers The buffers containing the message part to send.

        @return The number of bytes sent from the buffers.

        @throws system_error Thrown on failure.
    */
    template <class WebsocketStream, class ConstBufferSequence>
    static std::size_t write_some(WebsocketStream& stream, bool fin,
                                  ConstBufferSequence const& buffers)
    {
        return stream.ws().write_some(fin, buffers);
    }

    /** Write some message data.

        This function is used to send part of a message.

        The call blocks until one of the following is true:

        @li The message data is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        If this is the beginning of a new message, the message opcode
        will be set to text or binary based on the current setting of
        the @ref binary (or @ref text) option. The actual payload sent
        may be transformed as per the WebSocket protocol settings.

        @param fin `true` if this is the last part of the message.

        @param buffers The buffers containing the message part to send.

        @param ec Set to indicate what error occurred, if any.

        @return The number of bytes sent from the buffers.

        @return The number of bytes consumed in the input buffers.
    */
    template <class WebsocketStream, class ConstBufferSequence>
    static std::size_t write_some(WebsocketStream& stream, bool fin,
                                  ConstBufferSequence const& buffers,
                                  error_code& ec)
    {
        return stream.ws().write_some(fin, buffers, ec);
    }

    /** Write some message data asynchronously.

        This function is used to asynchronously write part of a message.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The message data is written.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's
        `async_write_some` function. The program must ensure that no other
        calls to @ref write, @ref write_some, @ref async_write, or
        @ref async_write_some are performed until this operation completes.

        If this is the beginning of a new message, the message opcode
        will be set to text or binary based on the current setting of
        the @ref binary (or @ref text) option. The actual payload sent
        may be transformed as per the WebSocket protocol settings.

        @param fin `true` if this is the last part of the message.

        @param buffers The buffers containing the message part to send.
        The implementation will make copies of this object
        as needed, but ownership of the underlying memory is not
        transferred. The caller is responsible for ensuring that
        the memory locations pointed to by buffers remains valid
        until the completion handler is called.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec,           // Result of operation
            std::size_t bytes_transferred   // Number of bytes sent from the
                                            // buffers. If an error occurred,
                                            // this will be less than the
       buffer_size.
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template <class WebsocketStream, class ConstBufferSequence,
              BOOST_BEAST_ASYNC_TPARAM2 WriteHandler =
                  net::default_completion_token_t<executor_type>>
    static BOOST_BEAST_ASYNC_RESULT2(WriteHandler)
        async_write_some(WebsocketStream& stream, bool fin,
                         ConstBufferSequence const& buffers,
                         WriteHandler&& handler =
                             net::default_completion_token_t<executor_type>{})
    {
        return stream.ws().async_write_some(fin, buffers, handler);
    }
};

}    // namespace websocket
}    // namespace beast
}    // namespace boost

using websocket_stream_base = boost::beast::websocket::websocket_stream_base;

#endif    // !WEBSOCKET_STREAM_BASE_HPP
