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
#include <c4m/linux/find_camera.hpp>

#include <c4m/split_capture_on_nalu_type.hpp>

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
         c4m::linux::camera2<c4m::default_features> camera;
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
        }
    }

private:

    ba::io_service& m_io_service;
    ba::ip::tcp::acceptor m_acceptor;

};

/// Write data to the socket in the same format as given for the
/// write_custom_capture_v2(...) function in
/// examples/write_file/write_file.cpp
class tcp_server_v2
{
public:

    tcp_server_v2(ba::io_service& io_service, const std::string& camera)
        : m_io_service(io_service),
          m_acceptor(io_service,
                     ba::ip::tcp::endpoint(ba::ip::tcp::v4(), 54321)),
          m_camera(camera)
    {
        do_accept();
    }

private:

    void do_stream(ba::ip::tcp::socket client)
    {
         c4m::linux::camera2<c4m::default_features> camera;
         camera.open(m_camera.c_str());

         std::cout << "Pixelformat: " << camera.pixelformat() << std::endl;

         std::cout << "Requesting resolution: " << std::endl;
         camera.request_resolution(400,500);
         std::cout << "w = " << camera.width() << " "
                   << "h = " << camera.height() << std::endl;

         // The time stamp of the previous captured NALU (needed to
         // calculate difference between two NALUs)
         uint64_t previous_timestamp = 0;

         // Write header
         write_to_socket<uint32_t>(client, camera.width());
         write_to_socket<uint32_t>(client, camera.height());

         camera.start_streaming();

         // Counts the number of NALUs
         uint32_t frames = 0;
         while(1)
         {
             auto data = camera.capture();
             assert(data);

             uint32_t diff_timestamp = data.m_timestamp - previous_timestamp;
             if (data.m_timestamp < previous_timestamp)
             {
                 std::cout << "Drop capture data due to timestamp issue"
                           << std::endl;
                 continue;
             }

             assert(data.m_timestamp >= previous_timestamp);
             previous_timestamp = data.m_timestamp;

             auto split_captures = c4m::split_capture_on_nalu_type(data);

             for (const auto& c : split_captures)
             {
                 std::cout << frames << ": " << c << " diff_timestamp = "
                           << diff_timestamp << std::endl;

                 auto nalus = n4lu::to_annex_b_nalus(c.m_data, c.m_size);

                 for(const auto& nalu : nalus)
                 {
                     std::cout << "  " << nalu << std::endl;
                     assert(nalu);
                 }

                 write_to_socket<uint64_t>(client, c.m_timestamp);
                 write_to_socket<uint32_t>(client, c.m_size);
                 write_to_socket(client, c.m_data, c.m_size);

                 ++frames;
             }
         }
    }


    void do_accept()
    {
        auto server_endpoint = m_acceptor.local_endpoint();
        std::cout << "Server on: " << server_endpoint << std::endl;
        std::cout << "Camera: " << m_camera << std::endl;

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
        }
    }

private:

    ba::io_service& m_io_service;
    ba::ip::tcp::acceptor m_acceptor;
    std::string m_camera;

};


int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    try
    {
        ba::io_service io_service;

        auto camera = c4m::linux::find_camera();

        if (camera.empty())
        {
            std::cerr << "Error: No h264 capable camera found." << std::endl;
            return 1;
        }


        tcp_server_v2 s(io_service, camera);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
