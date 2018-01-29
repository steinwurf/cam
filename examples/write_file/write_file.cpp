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

#include <endian/big_endian.hpp>
#include <nalu/to_annex_b_nalus.hpp>


#include <cam/camera.hpp>
#include <cam/linux/find_camera.hpp>
#include <cam/split_capture_on_nalu_type.hpp>

/// Helper to write types to the file
template<class T>
void write_to_file(std::fstream& file, T value)
{
    static char buffer[sizeof(T)];

    endian::big_endian::put<T>(value, (uint8_t*) buffer);

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

    cam::camera camera;
    camera.try_open(device);

    std::cout << "Pixelformat: " << camera.pixelformat() << std::endl;

    camera.try_request_resolution(800,600);
    std::cout << "w = " << camera.width() << " "
              << "h = " << camera.height() << std::endl;

    if (camera.has_h264_codec_control())
    {
        auto& control = camera.h264_codec_control();
        control.try_request_i_frame_period(1000);
        std::cout << "i_frame = " << control.i_frame_period() << std::endl;
    }

    camera.try_start_streaming();

    if (camera.has_h264_codec_control())
    {
        auto& control = camera.h264_codec_control();
        control.try_request_bitrates(100000,100000);
    }

    uint32_t frames = 0;
    while(frames < 500)
    {
        auto data = camera.try_capture();
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
///   |                       SPS timestamp                           |
///   |                                                               |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                        SPS NALU size                          |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                          SPS NALU                             |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                       PPS timestamp                           |
///   |                                                               |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                        PPS NALU size                          |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                          PPS NALU                             |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                     Aggregate timestamp                       |
///   |                                                               |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                      Aggregate NALU size                      |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///   |                                                               |
///   :                    Aggregate NALU data                        :
///   |                                                               |
///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
///
/// @endcode
///
/// The three fields: timestamp, NALU size, NALU data will repeat until the
/// end of file.
///
/// Field     | size
/// ----------+----------
/// timestamp | uint64_t
/// NALU size | uint32_t
/// NALU data | NALU size
///
/// The first NALU will be the SPS, the second NALU will be the PPS which
/// is the followed by the video NALUs. The SPS and PPS NALUs will
/// periodically repeat.
///
/// The aggregate NALU data contains the Annex B NALU data i.e. containing
/// the Annex B start codes. So the first byte of the NALU data will be the
/// 3 or 4 byte Annex B start code followed by the NALU header. The
/// aggregate NALU data may contain 1 or more NALUs (SPS and PPS are always
/// sent in non-aggregate fashion).
///
/// All multi-byte fields are written in Big Endian format.
///
void write_custom_capture(const char* device, const char* filename)
{
    std::fstream capture_file(filename, std::ios::out |
                              std::ios::binary | std::ios::trunc);

    std::cout << "Custom capture file: " << filename << std::endl;
    std::cout << "Device: " << device << std::endl;

    using camera_type = cam::linux::camera;
    camera_type camera;

    camera.set_trace_callback(
        [](const std::string& zone, const std::string& msg)
        {
            std::cout << zone << ":\n" << msg << std::endl;
        });

    camera.try_open(device);

    std::cout << "Pixelformat: " << camera.pixelformat() << std::endl;

    std::cout << "Requesting resolution: " << std::endl;
    camera.try_request_resolution(400,600);
    std::cout << "w = " << camera.width() << " "
              << "h = " << camera.height() << std::endl;

    // The time stamp of the previous captured NALU (needed to
    // calculate difference between two NALUs)
    uint64_t previous_timestamp = 0;

    camera.try_start_streaming();

    uint32_t frames = 0;
    while(frames < 100)
    {
        auto data = camera.try_capture();
        assert(data);

        if (data.m_timestamp < previous_timestamp)
        {
            std::cout << "Drop capture data due to timestamp issue" << std::endl;
            continue;
        }

        assert(data.m_timestamp >= previous_timestamp);
        previous_timestamp = data.m_timestamp;

        auto split_captures = cam::split_capture_on_nalu_type(data);

        for (const auto& c : split_captures)
        {

            auto nalus = nalu::to_annex_b_nalus(c.m_data, c.m_size);

            write_to_file<uint64_t>(capture_file, c.m_timestamp);
            write_to_file<uint32_t>(capture_file, c.m_size);
            write_to_file(capture_file, c.m_data, c.m_size);
        }

        ++frames;
    }

    std::cout << "Custom capture file: " << filename << std::endl;
    std::cout << "Device: " << device << std::endl;

    std::cout << "w = " << camera.width() << " "
              << "h = " << camera.height() << std::endl;

}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    auto camera_file = cam::linux::find_camera();

    if (camera_file.empty())
    {
        std::cerr << "Error: No h264 capable camera found." << std::endl;
        return 1;
    }

    try
    {
        write_raw_capture(camera_file.c_str(), "raw_capture.h264");
        write_custom_capture(camera_file.c_str(), "custom_capture.h264");
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
