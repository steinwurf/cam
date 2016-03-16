// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <cstdint>
#include <fstream>
#include <string>
#include <boost/asio.hpp>

#include "rtp_camera.hpp"

int main()
{
    std::string ip = "10.0.0.113";
    std::string port = "554";
    std::string location = "rtpstream/config5=u";

    ba::io_service io_service;
    rtp_camera camera(&io_service, ip, port, location);
    camera.connect();
    camera.describe();
    camera.setup();
    camera.play();

    std::ofstream h264_file("stream.h264", std::ios::binary);
    for (uint32_t i = 0; i < 100; ++i)
    {
        std::vector<uint8_t> h264;
        uint32_t timestamp;
        camera.read(h264, timestamp);
        if (h264.size() != 0)
            h264_file.write((char*)h264.data(), h264.size());
    }
    h264_file.close();

    camera.teardown();

    return 0;
}
