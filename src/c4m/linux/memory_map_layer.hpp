// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <vector>

#include "memory_map_buffer.hpp"

namespace c4m
{
namespace linux
{
    template<class Super>
    class memory_map_layer : public Super
    {
    public:

        bool is_memory_map_complete() const
        {
            return m_buffers.size() > 0;
        }

        void memory_map_buffers(std::error_code& error)
        {
            assert(!error);

            v4l2_requestbuffers request;
            memset(&request, 0, sizeof(request));

            request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            request.memory = V4L2_MEMORY_MMAP;
            request.count = 3; // Should be a parameter

            Super::retry_ioctl(VIDIOC_REQBUFS, &request, error);

            if (error)
            {
                return;
            }

            // Driver may be out of memory:
            // https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-reqbufs.html
            if (request.count == 0)
            {
                error = c4m::error::unable_to_allocate_buffers;
                return;
            }

            std::vector<memory_map_buffer> buffers;
            buffers.reserve(request.count);

            const auto& fd = Super::file_descriptor();
            assert(fd);

            for (uint32_t i = 0; i < request.count; ++i)
            {
                v4l2_buffer buffer;
                memset(&buffer, 0, sizeof(v4l2_buffer));

                buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buffer.memory = V4L2_MEMORY_MMAP;
                buffer.index = i;

                Super::retry_ioctl(VIDIOC_QUERYBUF, &buffer, error);

                if (error)
                {
                    return;
                }

                uint8_t* data = (uint8_t*) ::mmap(
                    0, buffer.length, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd.native_handle(), buffer.m.offset);

                memory_map_buffer mapped_buffer(data, buffer.length);
                buffers.push_back(std::move(mapped_buffer));

                if (MAP_FAILED == data)
                {
                    error.assign(errno, std::generic_category());
                    return;
                }
            }

            m_buffers = buffers;
        }

        private:

            std::vector<memory_map_buffer> m_buffers;

    };
}
}
