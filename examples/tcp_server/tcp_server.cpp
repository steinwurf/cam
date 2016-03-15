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
#include <boost/program_options.hpp>


#include <sak/convert_endian.hpp>
#include <n4lu/to_annex_b_nalus.hpp>

#include <c4m/linux/linux.hpp>
#include <c4m/linux/camera.hpp>
#include <c4m/linux/find_camera.hpp>

#include <c4m/split_capture_on_nalu_type.hpp>

#include "get_option.hpp"

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
class tcp_server
{
public:

    tcp_server(
        ba::io_service& io_service,
        const std::string& camera_device,
        const bpo::variables_map& vm)
        : m_io_service(io_service),
          m_acceptor(io_service,
                     ba::ip::tcp::endpoint(ba::ip::tcp::v4(), 54321)),
          m_camera_device(camera_device),
          m_variables_map(vm)
    {
        do_accept();
    }

private:

    void do_accept()
    {
        auto server_endpoint = m_acceptor.local_endpoint();
        std::cout << "Server on: " << server_endpoint << std::endl;
        std::cout << "Camera: " << m_camera_device << std::endl;

        auto client_socket = ba::ip::tcp::socket(m_io_service);

        m_acceptor.accept(client_socket);

        // Counts the number of NALUs
        m_frames = 0;
        m_diff_timestamp = 0;
        m_previous_timestamp = 0;

        m_client = std::unique_ptr<ba::ip::tcp::socket>(
            new ba::ip::tcp::socket(std::move(client_socket)));

        m_camera = std::unique_ptr<c4m::linux::camera<c4m::default_features>>(
            new c4m::linux::camera<c4m::default_features>);
        m_camera->set_io_service(&m_io_service);

        m_camera->try_open(m_camera_device.c_str());

        std::cout << "Pixelformat: " << m_camera->pixelformat() << std::endl;

        std::cout << "Requesting resolution: " << std::endl;
        m_camera->try_request_resolution(400, 500);
        std::cout << "w = " << m_camera->width() << " "
                  << "h = " << m_camera->height() << std::endl;

        if (m_variables_map.count("i_frame_period"))
        {
            m_camera->try_request_i_frame_period(
                get_option<uint32_t>(m_variables_map, "i_frame_period"));
        }

        m_camera->try_start_streaming();

        if (m_variables_map.count("bitrate"))
        {
            auto bitrate = get_option<uint32_t>(m_variables_map, "bitrate");
            m_camera->try_request_bitrates(bitrate, bitrate);
        }

        m_camera->async_capture(std::bind(
            &tcp_server::do_stream, this, std::placeholders::_1));
    }

    void do_stream(boost::system::error_code error)
    {
        if (error)
        {
            m_client.reset();
            m_camera.reset();
            m_io_service.post([&]{do_accept();});
            /// @todo Fix so that it works later, now just crash and burn.
            assert(0);
            return;
        }
        do_read();
        m_camera->async_capture(std::bind(
            &tcp_server::do_stream, this, std::placeholders::_1));
    }

    void do_read()
    {
        assert(m_camera);
        auto data = m_camera->try_capture();
        assert(data);

        // check if the timestamp is valid.
        // The timestamp is expected to be higher than the last.
        // The Logitech 920c camera can some times output timeframes which
        // are 400 times higher than the previous. This have been observed
        // when keeping ones hand above the camera lense for ~30 seconds.
        // This is obviously bogus, and hence we check that the timestamp
        // is no more than 10 times larger than the last.
        if (data.m_timestamp < m_previous_timestamp)
        {
            return;
        }

        if (m_previous_timestamp != 0 &&
            data.m_timestamp / m_previous_timestamp > 10)
        {
            return;
        }

        assert(data.m_timestamp >= m_previous_timestamp);

        m_diff_timestamp = data.m_timestamp - m_previous_timestamp;
        m_previous_timestamp = data.m_timestamp;

        auto split_captures = c4m::split_capture_on_nalu_type(data);
        for (const auto& c : split_captures)
        {
            std::cout << m_frames << ": " << c << " diff_timestamp = "
                      << m_diff_timestamp << std::endl;

            auto nalus = n4lu::to_annex_b_nalus(c.m_data, c.m_size);

            for (const auto& nalu : nalus)
            {
                std::cout << "  " << nalu << std::endl;
                assert(nalu);
            }

            write_to_socket<uint64_t>(*m_client, c.m_timestamp);
            write_to_socket<uint32_t>(*m_client, c.m_size);
            write_to_socket(*m_client, c.m_data, c.m_size);

            ++m_frames;
        }
    }

private:

    ba::io_service& m_io_service;
    ba::ip::tcp::acceptor m_acceptor;
    std::string m_camera_device;
    bpo::variables_map m_variables_map;

    std::unique_ptr<ba::ip::tcp::socket> m_client;
    std::unique_ptr<c4m::linux::camera<c4m::default_features>> m_camera;

    // Counts the number of NALUs
    uint32_t m_frames = 0;
    uint32_t m_diff_timestamp = 0;
    uint32_t m_previous_timestamp = 0;
};

int main(int argc, char* argv[])
{
    // Declare the supported options.
    bpo::options_description description("Allowed options");
    description.add_options()
        ("help", "produce help message")
        ("i_frame_period", bpo::value<uint32_t>()->default_value(1000),
         "The period in milliseconds between two i-frames")
        // ("average_bitrate", bpo::value<uint32_t>(),
        //  "The average bitrate in bits/sec")
        ("bitrate", bpo::value<uint32_t>(),
         "The peak and average bitrate in bits/sec");

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

        auto camera = c4m::linux::find_camera();

        if (camera.empty())
        {
            std::cerr << "Error: No h264 capable camera found." << std::endl;
            return 1;
        }


        tcp_server s(io_service, camera, vm);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    }

    return 0;
}
