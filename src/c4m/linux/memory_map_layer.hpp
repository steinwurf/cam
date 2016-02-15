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
    /// Responsible for ensuring that we have memory mapped the buffers
    /// needed such that data can be exchanged between user-space and the
    /// driver.
    template<class Super>
    class memory_map_layer : public Super
    {
    public:

        void start_streaming(std::error_code& error)
        {
            assert(!error);
            assert(m_buffers.size() == 0);

            memory_map_buffers(error);
        }

        void stop_streaming(std::error_code& error)
        {
            assert(!error);

            // Reset the buffers struct
            m_buffers.resize(0);
        }

        /// Maps the buffers in the driver to user-space. If the mapping
        /// fails an eror is set.
        ///
        /// @param error The error_code will be set in case of a failure.
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

            // We wait with updating the state of the layer until we know
            // we are in a good state
            m_buffers = std::move(buffers);
        }

        /// @returns The number of buffers allocated
        uint32_t buffer_count() const
        {
            return m_buffers.size();
        }

        const memory_map_buffer& buffer(uint32_t index) const
        {
            assert(index < m_buffers.size());
            return m_buffers[index];
        }

    private:

        /// The memory mapped buffers
        std::vector<memory_map_buffer> m_buffers;

    };
}
}
