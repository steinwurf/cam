// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <cam/camera.hpp>

int main(int argc, char* argv[])
{
    if (argc != 2 || std::string(argv[1]) == "--help")
    {
        auto usage = "./uvc_test DEVICE";
        std::cout << usage << std::endl;
        return 0;
    }
    auto device = std::string(argv[1]);
    try
    {
        cam::camera c;
        c.set_trace_stdout();

        c.try_open(device);

        std::cout << "w = " << c.width() << " "
                  << "h = " << c.height() << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
