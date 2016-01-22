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

#include <c4m/linux/linux.hpp>
#include <c4m/linux/layers.hpp>

namespace ba = boost::asio;

class tcp_server
{
public:

    tcp_server(ba::io_service& io_service, uint16_t port)
        : m_io_service(io_service),
          m_acceptor(io_service,
                     ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port))
    {
        do_accept();
    }

private:

    void do_stream(ba::ip::tcp::socket client)
    {
        c4m::linux::camera cam;

        auto fd = c4m::linux::open("/dev/video1");

        v4l2_capability capability;
        memset(&capability, 0, sizeof(capability));
        c4m::linux::read_capability(fd, &capability);

        assert(c4m::linux::is_a_video_capture_device(capability));
        assert(c4m::linux::has_streaming_io_ioctls(capability));

        v4l2_format format = c4m::linux::read_format(fd);

        // Request format
        format.fmt.pix.width = 800;
        format.fmt.pix.height = 600;
        format.fmt.pix.pixelformat = v4l2_fourcc('H','2','6','4');

        c4m::linux::set_format(fd, &format);

        c4m::linux::print_v4l2_format(std::cout, format);

        uint32_t buffer_count =
            c4m::linux::request_memory_map_buffers(fd, 3);

        auto buffers = c4m::linux::memory_map_buffers(fd, buffer_count);

        assert(buffers.size() == buffer_count);

        c4m::linux::enqueue_buffers(fd, buffer_count);
        c4m::linux::start_streaming(fd);

        while(1)
        {
            auto buffer_info = c4m::linux::dequeue_buffer(fd);

            uint32_t index = buffer_info.index;
            uint32_t bytesused = buffer_info.bytesused;

            std::cout << "Dequeued index = " << index
                      << " bytes used = " << bytesused
                      << std::endl;

            assert(index < buffers.size());

            const auto& b = buffers[index];

            assert(b);

            ba::write(client, ba::buffer(b.data(), bytesused));

            // Re-enqueue that buffer
            c4m::linux::enqueue_buffer(fd, index);
            // ++frames;
        }

        // c4m::linux::stop_streaming(fd);

        // client.shutdown(ba::ip::tcp::socket::shutdown_both);
        // client.close();


    }


    void do_accept()
    {
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

        tcp_server s(io_service, 54321);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
