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

#include <sak/convert_endian.hpp>

#include <nalu/to_annex_b_nalu.hpp>
#include <nalu/to_annex_b_nalus.hpp>

namespace ba = boost::asio;

/// Helper to read types from the socket
template<class T>
T read_from_socket(ba::ip::tcp::socket& socket)
{
    static uint8_t data[sizeof(T)];

    ba::read(socket, ba::buffer(data, sizeof(T)));

    return sak::big_endian::get<T>(data);
}

// Helper to write raw binary data to the socket
void read_from_socket(ba::ip::tcp::socket& socket,
                     uint8_t* data, uint32_t size)
{
    ba::read(socket, ba::buffer(data, size));
}

int run_client(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    try
    {
        ba::io_service io_service;

        ba::ip::tcp::socket s(io_service);

        auto endpoint = ba::ip::tcp::endpoint(
            ba::ip::address_v4::from_string("127.0.0.1"), 54321);


        // For the captured data
        std::vector<uint8_t> buffer;

        // The time stamp of the previous captured frame (needed to
        // calculate sample time)
        uint64_t previous_timestamp = 0;

        s.connect(endpoint);

        // Counts the number of capture segments
        uint32_t frames = 0;
        while(1)
        {

            uint64_t timestamp = read_from_socket<uint64_t>(s);

            // The sample time (timestamp delta)
            uint64_t sample_time = timestamp - previous_timestamp;

            uint32_t size = read_from_socket<uint32_t>(s);

            buffer.resize(size);

            read_from_socket(s, buffer.data(), size);

            std::cout << frames << ": size = " << size << " "
                      << "timestamp = " << timestamp << " "
                      << "sampletime = " << sample_time << std::endl;

            auto nalus = nalu::to_annex_b_nalus(buffer.data(), buffer.size());

            for(const auto& nalu : nalus)
            {
                std::cout << "  " << nalu << std::endl;
                assert(nalu);
            }

            previous_timestamp = timestamp;
            ++frames;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}


int main(int argc, char* argv[])
{
    run_client(argc, argv);
}
