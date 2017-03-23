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
#include <thread>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <cam/linux/camera.hpp>
#include <cam/linux/linux.hpp>
#include <cam/linux/find_camera.hpp>

#include "cam_source.hpp"
#include "wurf_it_source_server.hpp"

namespace ba = boost::asio;
namespace bpo = boost::program_options;
namespace
{
    void control_thread(bool* running, std::shared_ptr<boost::asio::io_service> io_service)
    {
        std::cin.ignore();
        std::cout << "User Quit" << std::endl;
        *running = false;
        io_service->stop();
    }
}

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

    bool running = true;
    std::shared_ptr<boost::asio::io_service> io_service;
    std::cout << "Press Enter to Quit " << std::endl;
    std::thread t(control_thread, &running, io_service);
    while (running)
    {
        try
        {
            auto camera = cam::linux::find_camera();

            if (camera.empty())
            {
                std::cerr << "Error: No h264 capable camera found." << std::endl;
                return 1;
            }

            cam_source cam(io_service, camera, vm);

            wurf_it_source_server<cam_source> source_server(io_service, cam, 54321);
            source_server.set_on_error_callback(
                std::bind([io_service](){io_service->stop();}));
            source_server.start();
            std::cout << "Server started" << std::endl;
            io_service->run();
            std::cout << "Server stopped" << std::endl;
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    }
    t.join();

    return 0;
}
