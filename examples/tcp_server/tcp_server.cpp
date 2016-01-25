// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

#include <sak/convert_endian.hpp>
#include <n4lu/to_annex_b_nalus.hpp>

#include <c4m/linux/linux.hpp>
#include <c4m/linux/layers.hpp>

namespace ba = boost::asio;

/// Helper to write types to the socket
template<class T>
void write_to_socket(ba::ip::tcp::socket& socket, T value)
{
    static uint8_t data[sizeof(T)];

    sak::big_endian::put<T>(value, data);

    ba::write(socket, ba::buffer(data, sizeof(T)));
}

// Helper to write raw binary data to the socket
void write_to_socket(ba::ip::tcp::socket& socket,
                     const uint8_t* data, uint32_t size)
{
    ba::write(socket, ba::buffer(data, size));
}

/// Write data to the socket in the same format as given for the
/// write_custom_capture(...) function in
/// examples/write_file/write_file.cpp
class tcp_server
{
public:

    tcp_server(ba::io_service& io_service)
        : m_io_service(io_service),
          m_acceptor(io_service,
                     ba::ip::tcp::endpoint(ba::ip::tcp::v4(), 54321))
    {
        do_accept();
    }

private:

    void do_stream(ba::ip::tcp::socket client)
    {
         c4m::linux::camera2 camera;
         camera.open("/dev/video1");

         std::cout << "Pixelformat: " << camera.pixelformat() << std::endl;

         std::cout << "Requesting resolution: " << std::endl;
         camera.request_resolution(400,500);
         std::cout << "w = " << camera.width() << " "
                   << "h = " << camera.height() << std::endl;

         // Write header
         write_to_socket<uint32_t>(client, camera.width());
         write_to_socket<uint32_t>(client, camera.height());

         camera.start_streaming();

         // Counts the number of NALUs
         uint32_t nalu_count = 0;
         while(1)
         {
             auto data = camera.capture();
             assert(data);

             std::cout << data << std::endl;

             auto nalus = n4lu::to_annex_b_nalus(data.m_data, data.m_size);

             for(const auto& nalu : nalus)
             {
                 std::cout << "  " << nalu_count << ": " << nalu << std::endl;
                 assert(nalu);

                 write_to_socket<uint64_t>(client, data.m_timestamp);

                 write_to_socket<uint32_t>(client, nalu.m_size);
                 write_to_socket(client, nalu.m_data, nalu.m_size);
                 ++nalu_count;
             }
         }
    }


    void do_accept()
    {
        auto server_endpoint = m_acceptor.local_endpoint();
        std::cout << "Server on: " << server_endpoint << std::endl;

        while(1)
        {
            auto client_socket = ba::ip::tcp::socket(m_io_service);

            m_acceptor.accept(client_socket);

            try
            {
                do_stream(std::move(client_socket));
            }
            catch (std::exception& e)
            {
                std::cerr << "Exception: " << e.what() << "\n";
            }

            static int i = 0;
            ++i;

            if (i > 2)
                return;
        }
    }

private:

    ba::io_service& m_io_service;
    ba::ip::tcp::acceptor m_acceptor;

};

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    try
    {
        ba::io_service io_service;

        tcp_server s(io_service);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
