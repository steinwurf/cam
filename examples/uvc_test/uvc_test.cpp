// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <cam/camera.hpp>

int main()
{
    try
    {
        cam::camera s;
        s.set_trace_stdout();

        s.try_open("/dev/video2");

        std::cout << "w = " << s.width() << " "
                  << "h = " << s.height() << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
