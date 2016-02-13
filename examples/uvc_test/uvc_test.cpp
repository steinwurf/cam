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

    c4m::linux::camera2<c4m::default_features> s;

    // using stack = c4m::throw_if_error_layer<
    //     c4m::linux::xu_query<
    //     c4m::linux::create_usb_device<
    //     c4m::linux::create_udev_device<
    //     c4m::linux::final_layer2>>>>;

    // stack s;

    s.try_open("/dev/video1");
    //s.try_request_resolution(800,600);
    std::cout << "w = " << s.width() << " "
              << "h = " << s.height() << std::endl;


    //s.try_open("/sys/class/video4linux/video1");

    std::error_code error;
    s.config_query(error);

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
