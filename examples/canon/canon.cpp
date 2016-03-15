// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <sak/convert_endian.hpp>
#include <c4m/split_capture_on_nalu_type.hpp>

#include "get_option.hpp"
#include "rtp_camera.hpp"

namespace ba = boost::asio;
namespace bpo = boost::program_options;

// Helper to write raw binary data to the socket
void write_to_socket(ba::ip::tcp::socket& socket,
                     const uint8_t* data, uint32_t size)
{
    ba::write(socket, ba::buffer(data, size));
}

/// Helper to write types to the socket
template<class T>
void write_to_socket(ba::ip::tcp::socket& socket, T value)
{
    static uint8_t data[sizeof(T)];

    sak::big_endian::put<T>(value, data);
    write_to_socket(socket, data, sizeof(T));
}

/// Write data to the socket in the same format as given for the
/// write_custom_capture(...) function in
/// examples/write_file/write_file.cpp
class tcp_server_rtp
{
public:
    tcp_server_rtp(
        ba::io_service& io_service,
        const bpo::variables_map& vm)
        : m_io_service(io_service),
          m_acceptor(io_service,
                     ba::ip::tcp::endpoint(ba::ip::tcp::v4(), 54321)),
          m_variables_map(vm)
    {
        do_accept();
    }

private:

    void do_accept()
    {
        auto server_endpoint = m_acceptor.local_endpoint();
        std::cout << "Server on: " << server_endpoint << std::endl;

        while(1)
        {
            auto client_socket = ba::ip::tcp::socket(m_io_service);

            m_acceptor.accept(client_socket);
            std::cout << "found one!" << std::endl;
            try
            {
                do_stream(std::move(client_socket));
            }
            catch (std::exception& e)
            {
                std::cerr << "Exception: " << e.what() << "\n";
            }
        }
    }

    void do_stream(ba::ip::tcp::socket client)
    {
        rtp_camera camera(m_io_service);
        auto ip = get_option<std::string>(m_variables_map, "ip");
        auto port = get_option<std::string>(m_variables_map, "port");
        camera.try_open(ip, port);

        // The time stamp of the previous captured NALU (needed to
        // calculate difference between two NALUs)
        uint64_t previous_timestamp = 0;

        camera.try_start_streaming();

        // Counts the number of NALUs
        uint32_t frames = 0;
        uint32_t diff_timestamp = 0;
        while(1)
        {
            uint32_t timestamp;
            std::vector<uint8_t> h264;
            camera.try_capture(h264, timestamp);
            if (h264.size() == 0)
                continue;

            std::cout << "data " << h264.m_size << std::endl;
            std::cout << (uint32_t)h264.m_data[0] << std::endl;
            std::cout << (uint32_t)h264.m_data[1] << std::endl;
            std::cout << (uint32_t)h264.m_data[2] << std::endl;
            std::cout << (uint32_t)h264.m_data[3] << std::endl;

            diff_timestamp = timestamp - previous_timestamp;
            previous_timestamp = timestamp;

            auto split_captures =
                c4m::split_capture_on_nalu_type<c4m::capture_data>(data);
            for (const auto& c : split_captures)
            {
                std::cout << frames << ": " << c << " diff_timestamp = "
                          << diff_timestamp << std::endl;

                write_to_socket<uint64_t>(client, c.m_timestamp);
                write_to_socket<uint32_t>(client, c.m_size);
                write_to_socket(client, c.m_data, c.m_size);

                ++frames;
            }
        }
    }

private:

    ba::io_service& m_io_service;
    ba::ip::tcp::acceptor m_acceptor;
    bpo::variables_map m_variables_map;
};

int main(int argc, char* argv[])
{
    // Declare the supported options.
    bpo::options_description description("Allowed options");
    description.add_options()
        ("ip", bpo::value<std::string>(),
         "IP of the ip camera.")
        ("port", bpo::value<std::string>(),
         "Port of the ip camera.")
        ("help", "produce help message");

    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(argc, argv, description), vm);
    bpo::notify(vm);

    if (vm.count("help"))
    {
        std::cout << description << std::endl;
        return 1;
    }

    while(1)
    {
        try
        {
            ba::io_service io_service;

            tcp_server_rtp s(io_service, vm);

            io_service.run();
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    }

    return 0;
}

