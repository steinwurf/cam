// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <cam/linux/camera.hpp>

int run_uvc(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    cam::linux::camera<cam::default_features> s;

    s.try_open("/dev/video1");

    std::cout << "w = " << s.width() << " "
              << "h = " << s.height() << std::endl;

    return 0;
}


int main(int argc, char* argv[])
{
    try
    {
        run_uvc(argc, argv);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
