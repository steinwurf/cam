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

namespace ba = boost::asio;

template<class Source>
class wurf_it_source_server
{

public:

    wurf_it_source_server(ba::io_service* io, Source source, uint16_t port):
        m_acceptor(*io, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port)),
        m_socket(*io),
        m_source(source)
    { }

    void start()
    {
        m_acceptor.async_accept(m_socket, std::bind(
            &wurf_it_source_server::accept_handler,
            this,
            std::placeholders::_1));
    }

    void stop()
    {
        if (m_started)
        {
            m_source.stop();
        }
    }

private:

    void accept_handler(const boost::system::error_code& error)
    {
        if (error)
        {
            return;
        }
        m_source.init();
        m_source.start(std::bind(
            &wurf_it_source_server::handle_data,
            this,
            std::placeholders::_1,
            std::placeholders::_2));
        m_started = true;
    }

    void handle_data(const std::vector<uint8_t>& data, uint64_t timestamp)
    {
        send(timestamp);
        send<uint32_t>(data.size());
        send(data);
    }

    template<class T>
    void send(T value)
    {
        std::vector<uint8_t> data(sizeof(T));

        sak::big_endian::put<T>(value, data.data());
        send(data);
    }

    void send(const std::vector<uint8_t>& data)
    {
        auto d = std::make_shared<const std::vector<uint8_t>>(data);
        send(d);
    }

    void send(std::shared_ptr<const std::vector<uint8_t>> data)
    {
        ba::async_write(m_socket, ba::buffer(data->data(), data->size()),
            std::bind(&wurf_it_source_server::on_send_finished, this, data));
    }

    void on_send_finished(
        std::shared_ptr<const std::vector<uint8_t>> data) const
    {
        data.reset();
    }

private:

    ba::ip::tcp::acceptor m_acceptor;
    ba::ip::tcp::socket m_socket;
    Source m_source;

    bool m_started = false;
};
