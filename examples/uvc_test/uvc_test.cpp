// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <c4m/linux/layers.hpp>

int run_uvc(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    using stack = c4m::throw_if_error_layer<
        c4m::linux::create_udev_device<
        c4m::linux::final_layer2>>;

    stack s;

    s.try_open("/dev/video1");
    //s.try_open("/sys/class/video4linux/video1");


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
