// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <endian/big_endian.hpp>
#include <cam/split_capture_on_nalu_type.hpp>

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

    endian::big_endian::put<T>(value, data);
    write_to_socket(socket, data, sizeof(T));
}

/// Write data to the socket in the same format as given for the
/// write_custom_capture(...) function in
/// examples/write_file/write_file.cpp
class tcp_server_rtp
{
public:
    tcp_server_rtp(
        ba::io_service* io_service,
        const bpo::variables_map& vm)
        : m_io_service(io_service),
          m_acceptor(*m_io_service,
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
            auto client_socket = ba::ip::tcp::socket(*m_io_service);

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
        auto ip = get_option<std::string>(m_variables_map, "ip");
        auto port = get_option<std::string>(m_variables_map, "port");
        std::string location = "rtpstream/config5=u";
        rtp_camera camera(m_io_service, ip, port, location);

        camera.connect();
        camera.describe();
        camera.setup();
        camera.play();

        // Counts the number of NALUs
        uint32_t frames = 0;

        std::vector<uint8_t> h264;
        uint32_t timestamp;
        while(1)
        {
            h264.clear();
            camera.read(h264, timestamp);
            if (h264.size() == 0)
                continue;

            auto capture_data = cam::capture_data(
                h264.data(), h264.size(), timestamp);

            auto split_captures = cam::split_capture_on_nalu_type(capture_data);
            for (const auto& c : split_captures)
            {
                write_to_socket<uint64_t>(client, c.m_timestamp);
                write_to_socket<uint32_t>(client, c.m_size);
                write_to_socket(client, c.m_data, c.m_size);

                ++frames;
            }
        }
        camera.teardown();
    }

private:

    ba::io_service* m_io_service;
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

            tcp_server_rtp s(&io_service, vm);

            io_service.run();
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    }

    return 0;
}

