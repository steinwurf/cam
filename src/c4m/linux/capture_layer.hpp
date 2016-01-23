// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <vector>



namespace c4m
{
namespace linux
{
    template<class Super>
    class capture_layer : public Super
    {
    public:


        capture_data capture(std::error_code& error)
        {
            assert(!error);

            if (Super::is_memory_map_complete())
            {
                Super::memory_map_buffers(error);

                if (error)
                {
                    return capture_data();
                }
            }

            if (Super::is_buffers_enqueued())
            {
                // If this is the initial capture we enqueue all buffers
                Super::enqueue_buffers(fd, error);

                if (error)
                {
                    return capture_data();
                }

                // Start stream
                uint32_t type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                Super::retry_ioctl(VIDIOC_STREAMON, &type, error);

                if (error)
                {
                    return capture_data();
                }
            }
            else
            {
                // We re-enqueue the previously dequeued buffer
                Super::enqueue_buffer(m_dequeued_index, error);

                if (error)
                {
                    return capture_data();
                }
            }

        //     // This call will block
        //     auto buffer_info = c4m::linux::dequeue_buffer(fd, error);

        //     if (error)
        //         return capture_data();

        //     uint32_t index = buffer_info.index;
        //     uint32_t bytesused = buffer_info.bytesused;
        //     uint64_t timestamp = timestamp_to_micro_seconds(buffer_info);

        //     const auto& buffer = Super::buffer(index);

        //     capture_data data;
        //     data.m_data = buffer.data();
        //     data.m_size = bytesused;
        //     data.m_timestamp = timestamp;

        //     return data;
        // }



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

        // capture_data capture(std::error_code& error)
        // {
        //     const auto& fd = Super::file_descriptor();
        // //     assert(fd);

        // }

        // void fill_buffer(uint8_t* data, std::error_code& error)
        // {
        //     assert(data);

        //     const auto& fd = Super::file_descriptor();
        //     assert(fd);

        //     auto buffer_info = c4m::linux::dequeue_buffer(fd, error);

        //     if (error)
        //         return;

        //     uint32_t index = buffer_info.index;

        //     m_bytesused = buffer_info.bytesused;
        //     m_timestamp = timestamp_to_micro_seconds(buffer_info);

        //     assert(index < buffers.size());

        //     const auto& b = buffers[index];

        // }



    private:

        std::vector<memory_map_buffer> m_buffers;

    };
}
}
