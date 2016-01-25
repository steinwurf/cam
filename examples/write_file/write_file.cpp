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
#include <sstream>

#include <sak/convert_endian.hpp>
#include <n4lu/to_annex_b_nalus.hpp>

#include <c4m/linux/linux.hpp>
#include <c4m/linux/layers.hpp>

#include "hexdump.hpp"
//#include "annexb.hpp"


/// Helper to write types to the file
template<class T>
void write_to_file(std::fstream& file, T value)
{
    static char buffer[sizeof(T)];

    sak::big_endian::put<T>(value, (uint8_t*) buffer);

    file.write(buffer, sizeof(T));
}

// Helper to write raw binary data to the file
void write_to_file(std::fstream& file, const uint8_t* data, uint32_t size)
{
    file.write((const char*) data, size);
}

/// Writes a raw capture file (pure Annex B NALU data) taken directly from
/// the camera.
///
/// Such a file can be played in vlc with:
///
///    cvlc raw_capture.h264 --demux h264
///
void write_raw_capture(const char* device, const char* filename)
{
    std::fstream capture_file(filename, std::ios::out |
                              std::ios::binary | std::ios::trunc);

    std::cout << "Raw capture file: " << filename << std::endl;
    std::cout << "Device: " << device << std::endl;

    c4m::linux::camera2 camera;
    camera.open(device);

    std::cout << "Pixelformat: " << camera.pixelformat() << std::endl;

    std::cout << "Requesting resolution: " << std::endl;
    camera.request_resolution(400,500);
    std::cout << "w = " << camera.width() << " "
              << "h = " << camera.height() << std::endl;

    camera.start_streaming();

    uint32_t frames = 0;
    while(frames < 500)
    {
        auto data = camera.capture();
        std::cout << data << std::endl;

        capture_file.write((const char*) data.m_data, data.m_size);

        ++frames;
    }
}

/// Writes a custom capture file with the following binary format:
///
/// @code
///
///    0                   1                   2                   3
///    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                           width                               |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                           height                              |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                         timestamp                             |
///   |                                                               |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                         NALU size                             |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                                                               |
///   :                         NALU data                             :
///   |                                                               |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///
/// @endcode
///
/// The three fields: timestamp, NALU size, NALU data will repeat until the
/// end of file.
///
/// Field     | size
/// ---------------
/// width     | uint32_t
/// height    | uint32_t
/// timestamp | uint64_t
/// NALU size | uint32_t
/// NALU data | NALU size
///
/// The first NALU will be the SPS, the second NALU will be the PPS which
/// is the followed by the video NALUs. The SPS and PPS NALUs will
/// periodically repeat.
///
/// The NALU data contains the Annex B NALU data i.e. containing the Annex
/// B start codes. So the first byte of the NALU data will be the 3 or 4
/// byte Annex B start code followed by the NALU header.
///
/// All multi-byte fields are written in Big Endian format.
///
void write_custom_capture(const char* device, const char* filename)
{
    std::fstream capture_file(filename, std::ios::out |
                              std::ios::binary | std::ios::trunc);

    std::cout << "Custom capture file: " << filename << std::endl;
    std::cout << "Device: " << device << std::endl;

    c4m::linux::camera2 camera;
    camera.open(device);

    std::cout << "Pixelformat: " << camera.pixelformat() << std::endl;

    std::cout << "Requesting resolution: " << std::endl;
    camera.request_resolution(400,500);
    std::cout << "w = " << camera.width() << " "
              << "h = " << camera.height() << std::endl;

    // The time stamp of the previous captured NALU (needed to
    // calculate difference between two NALUs)
    uint64_t previous_timestamp = 0;

    // Write header
    write_to_file<uint32_t>(capture_file, camera.width());
    write_to_file<uint32_t>(capture_file, camera.height());

    camera.start_streaming();

    uint32_t frames = 0;
    while(frames < 500)
    {
        auto data = camera.capture();
        assert(data);

        uint32_t diff_timestamp = data.m_timestamp - previous_timestamp;
        assert(data.m_timestamp >= previous_timestamp);

        previous_timestamp = data.m_timestamp;

        std::cout << data << " diff_timestamp = "
                  << diff_timestamp << std::endl;

        auto nalus = n4lu::to_annex_b_nalus(data.m_data, data.m_size);

        for(const auto& nalu : nalus)
        {
            std::cout << "  " << nalu << std::endl;
            assert(nalu);

            write_to_file<uint64_t>(capture_file, data.m_timestamp);

            write_to_file<uint32_t>(capture_file, nalu.m_size);
            write_to_file(capture_file, nalu.m_data, nalu.m_size);
        }

        ++frames;
    }
}


int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    try
    {
        write_raw_capture("/dev/video1", "raw_capture.h264");
        write_custom_capture("/dev/video1", "custom_capture.h264");
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
