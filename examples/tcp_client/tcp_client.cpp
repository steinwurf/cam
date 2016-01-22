// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <boost/asio.hpp>

namespace ba = boost::asio;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
    std::fstream capture_file("capture.h264", std::ios::out |
                              std::ios::binary | std::ios::trunc);
    try
    {


        ba::io_service io_service;

        ba::ip::tcp::socket s(io_service);

        auto endpoint = ba::ip::tcp::endpoint(
            ba::ip::address_v4::from_string("127.0.0.1"), 54321);

        s.connect(endpoint);

        char data[max_length];

        uint32_t read = 0;

        while(read < 10000000)
        {
            size_t data_size = s.read_some(ba::buffer(data, max_length));

            std::cout << "Read: " << data_size << std::endl;
            std::cout << "Total: " << read << std::endl;

            capture_file.write(data, data_size);

            read += data_size;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    capture_file.close();

    return 0;
}
