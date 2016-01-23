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

#include <c4m/linux/linux.hpp>
#include <c4m/linux/layers.hpp>
#include <c4m/annex_b_find_nalus.hpp>

#include "hexdump.hpp"
#include "annexb.hpp"



void write_raw_capture(const uint8_t* data, uint32_t size, uint32_t frame_number)
{
    std::stringstream ss;
    ss << "annex_b_raw_capture_" << frame_number << ".h264";

    std::fstream file(ss.str(), std::ios::out |
                      std::ios::binary | std::ios::trunc);

    file.write(reinterpret_cast<const char*>(data), size);

    file.close();

}


uint64_t timestamp_to_micro_seconds(const v4l2_buffer& buffer)
{

    auto clock_type = buffer.flags & V4L2_BUF_FLAG_TIMESTAMP_MASK;
    assert(clock_type == V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC);

    uint64_t time = (buffer.timestamp.tv_sec * 1000000) +
        buffer.timestamp.tv_usec;

    // @todo if clock type is V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN
    // we can do the timestamp ourselves with some chrono or such

    return time;
}

template<class T>
void write_to_file(std::fstream& file, T value)
{
    std::vector<uint8_t> data(sizeof(T));

    sak::big_endian::put<uint32_t>(value, data.data());

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void write_to_file(std::fstream& file, c4m::annex_b_nalu value)
{
    write_to_file(file, value.m_size);
    file.write(reinterpret_cast<const char*>(value.m_data), value.m_size);
}

void write_capture(const char* filename)
{
    // {
    // auto fd = c4m::linux::open("/dev/video1");
    // std::error_code error;
    // v4l2_format format = c4m::linux::read_format(fd,error);
    // std::cout << c4m::linux::v4l2_pixel_format_to_string(
    //     format.fmt.pix.pixelformat) << std::endl;


    // }
    {
        c4m::linux::camera2 camera2;
        std::error_code error;
        camera2.open("/dev/video1", error);
        c4m::linux::camera camera;
        camera.open("/dev/video1");
        std::cout << camera.pixelformat() << std::endl;
        std::cout << "Requesting resolution: " << std::endl;
        camera.request_resolution(400,500);
        std::cout << "w = " << camera.width() << " h = "
                  << camera.height() << std::endl;

        // camera.start_streaming();

        // uint32_t max_buffer_size = camera.max_buffer_size();
        // std::cout << max_buffer_size << std::endl;

        // std::vector<uint8_t> buffer(max_buffer_size);

    }
    std::cout << "=============" << std::endl;

        {
        c4m::linux::camera camera;
        camera.open("/dev/video1");
        std::cout << camera.pixelformat() << std::endl;
    }

        return;

    std::fstream capture_file(filename, std::ios::out |
                              std::ios::binary | std::ios::trunc);

    auto fd = c4m::linux::open("/dev/video1");

    v4l2_capability capability;
    memset(&capability, 0, sizeof(capability));

    c4m::linux::read_capability(fd, &capability);

    assert(c4m::linux::is_a_video_capture_device(capability));
    assert(c4m::linux::has_streaming_io_ioctls(capability));

    v4l2_format format = c4m::linux::read_format(fd);

    // Request format
    format.fmt.pix.width = 800;
    format.fmt.pix.height = 600;
    format.fmt.pix.pixelformat = v4l2_fourcc('H','2','6','4');

    c4m::linux::set_format(fd, &format);

    assert(format.fmt.pix.pixelformat == v4l2_fourcc('H','2','6','4'));

    write_to_file(capture_file, format.fmt.pix.width);
    write_to_file(capture_file, format.fmt.pix.height);

    c4m::linux::print_v4l2_format(std::cout, format);

    uint32_t buffer_count =
        c4m::linux::request_memory_map_buffers(fd, 3);

    auto buffers = c4m::linux::memory_map_buffers(fd, buffer_count);

    assert(buffers.size() == buffer_count);

    c4m::linux::enqueue_buffers(fd, buffer_count);
    c4m::linux::start_streaming(fd);

    uint64_t initial_timestamp = 0;
    uint64_t prev_timestamp = 0;
    uint32_t frames = 0;
    while(frames < 20)
    {
        auto buffer_info = c4m::linux::dequeue_buffer(fd);

        uint32_t index = buffer_info.index;
        uint32_t bytesused = buffer_info.bytesused;

        auto timestamp = timestamp_to_micro_seconds(buffer_info);

        if(frames == 0)
        {
            initial_timestamp = timestamp;
            prev_timestamp = timestamp;
        }

        uint64_t presentation_time = timestamp - initial_timestamp;

        std::cout << "Dequeued buffer index = " << index
                  << " bytes used = " << bytesused
                  << " timestamp = " << presentation_time
                  << " diff = " << (timestamp - prev_timestamp)
                  << std::endl;

        prev_timestamp = timestamp;

        assert(index < buffers.size());

        const auto& b = buffers[index];

        auto nalus = c4m::annex_b_find_nalus(b.data(), bytesused);

        for(const auto& n : nalus)
        {
            write_to_file(capture_file, presentation_time);
            write_to_file(capture_file, n);
        }


        //temp::next_nalu(b.data(), b.data() + b.size());


        assert(b);
        // capture_file.write(reinterpret_cast<const char*>(b.data()), bytesused);
        // write_raw_capture(b.data(), bytesused, frames);

        // auto out = temp::hexdump(sak::storage(b.data(),bytesused));
        //out.set_max_size(48);

        // std::cout << out << std::endl;


        // std::cout << std::hex << << (uint32_t)b.data()[0]
        //           << std::dec << std::endl;


        // Re-enqueue that buffer
        c4m::linux::enqueue_buffer(fd, index);
        ++frames;
    }

    capture_file.close();
}


int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    try
    {
        write_capture("capture.h264");
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
