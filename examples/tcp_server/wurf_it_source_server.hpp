// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <sak/convert_endian.hpp>

#include <boost/asio.hpp>

#include <cstdint>
#include <functional>
#include <iostream>
#include <endian/big_endian.hpp>

namespace ba = boost::asio;

template<class Source>
class wurf_it_source_server
{

public:

    using on_error_callback = std::function<void()>;

public:

    wurf_it_source_server(std::shared_ptr<boost::asio::io_service> io_service, Source& source, uint16_t port):
        m_io(io_service),
        m_source(source),
        m_port(port),
        m_acceptor(*m_io)
    { }

    void start()
    {
        boost::asio::ip::tcp::endpoint endpoint(ba::ip::tcp::v4(), m_port);
        try
        {
            m_acceptor.open(endpoint.protocol());
            m_acceptor.set_option(
                boost::asio::ip::tcp::acceptor::reuse_address(true));
            m_acceptor.bind(endpoint);
            m_acceptor.listen();
            setup_async_accept();
        }
        catch (const std::exception& e)
        {
            std::cout << "unable to bind" << std::endl;
        }
    }

    void setup_async_accept()
    {
        auto connection = std::make_shared<ba::ip::tcp::socket>(*m_io);
        m_acceptor.async_accept(*connection.get(), std::bind(
            &wurf_it_source_server::accept_handler,
            this,
            connection,
            std::placeholders::_1));
    }

    void stop()
    {
        m_acceptor.cancel();
        m_acceptor.close();
        if (m_started)
        {
            m_source.stop();
        }
    }

    void set_on_error_callback(on_error_callback callback)
    {
        m_on_error_callback = callback;
    }

private:

    void accept_handler(
        std::shared_ptr<ba::ip::tcp::socket> connection,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            m_source.init();
            m_source.start(std::bind(
                &wurf_it_source_server::handle_data,
                this,
                connection,
                std::placeholders::_1,
                std::placeholders::_2));
            m_socket = connection;
            m_started = true;
            std::cout << "Got connection!" << std::endl;
        }

        // if we got operation aborted, we assume that we should stop the sender
        // and not setup a new async accept.
        if (error == boost::asio::error::operation_aborted)
            return;

        setup_async_accept();
    }

    void handle_data(std::shared_ptr<ba::ip::tcp::socket> connection,
        const std::vector<uint8_t>& data, uint64_t timestamp)
    {
        send(connection, timestamp);
        send<uint32_t>(connection, data.size());
        send(connection, data);
    }

    template<class T>
    void send(std::shared_ptr<ba::ip::tcp::socket> connection, T value)
    {
        std::vector<uint8_t> data(sizeof(T));
        endian::big_endian::put<T>(value, data.data());
        send(connection, data);
    }

    void send(std::shared_ptr<ba::ip::tcp::socket> connection,
        const std::vector<uint8_t>& data)
    {
        auto d = std::make_shared<const std::vector<uint8_t>>(data);
        send(connection, d);
    }

    void send(std::shared_ptr<ba::ip::tcp::socket> connection,
        std::shared_ptr<const std::vector<uint8_t>> data)
    {

        ba::async_write(*connection.get(), ba::buffer(data->data(), data->size()),
            std::bind(&wurf_it_source_server::on_send_finished, this,
                connection, data,
                std::placeholders::_1, std::placeholders::_2));
    }

    void on_send_finished(
        std::shared_ptr<ba::ip::tcp::socket> connection,
        std::shared_ptr<const std::vector<uint8_t>> data,
        const boost::system::error_code& error,
        std::size_t transferred)
    {
        (void) transferred;
        (void) connection;
        data.reset();

        if (!error)
            return;

        m_source.stop();

        if (error == boost::asio::error::operation_aborted ||
            error == boost::asio::error::eof)
        {
            return;
        }

        if (error && m_on_error_callback)
        {
            if (m_socket->is_open())
            {
                boost::system::error_code ignored;
                m_socket->shutdown(
                    boost::asio::ip::tcp::socket::shutdown_both, ignored);
                m_socket->close();
            }
            m_on_error_callback();
        }
    }

private:

    std::shared_ptr<boost::asio::io_service> m_io;
    Source& m_source;
    uint16_t m_port;
    ba::ip::tcp::acceptor m_acceptor;


    std::shared_ptr<ba::ip::tcp::socket> m_socket;
    bool m_started = false;

    on_error_callback m_on_error_callback;
};
