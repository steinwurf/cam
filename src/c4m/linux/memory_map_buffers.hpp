// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "retry_ioctl.hpp"
#include "memory_map_buffer.hpp"

#include <sys/mman.h>
#include <vector>

namespace c4m
{
namespace linux
{
    /// Maps the memory from the v4l2 driver to user-space.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    std::error_code error;
    ///    auto buffers =
    ///        c4m::linux::memory_map_buffers(fd, buffer_count, error);
    ///
    ///    if(error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    /// Docs: https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-reqbufs.html
    ///
    inline std::vector<memory_map_buffer>
    memory_map_buffers(const scoped_file_descriptor& fd,
                       uint32_t allocated_buffers,
                       std::error_code& error)
    {
        assert(fd);
        assert(allocated_buffers > 0);

        std::vector<memory_map_buffer> buffers;
        buffers.reserve(allocated_buffers);

        for (uint32_t i = 0; i < allocated_buffers; ++i)
        {
            struct v4l2_buffer buffer = {0};

            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_MMAP;
            buffer.index = i;

            retry_ioctl(fd, VIDIOC_QUERYBUF, &buffer, error);

            if (error)
            {
                return std::vector<memory_map_buffer>();
            }

            uint8_t* data = (uint8_t*) ::mmap(0, buffer.length,
                                              PROT_READ | PROT_WRITE,
                                              MAP_SHARED, fd.native_handle(),
                                              buffer.m.offset);

            memory_map_buffer mapped_buffer(data, buffer.length);
            buffers.push_back(std::move(mapped_buffer));

            if (MAP_FAILED == data)
            {
                error.assign(errno, std::generic_category());
            }
            else
            {
                error.assign(0, std::generic_category());
            }

            if (error)
            {
                return std::vector<memory_map_buffer>();
            }

        }

        return buffers;
    }

    /// Calls memory_map_buffers(...) with an error_code and throws an
    /// exception if an error is set.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    auto buffers = c4m::linux::memory_map_buffers(fd, buffer_count);
    ///
    inline std::vector<memory_map_buffer>
    memory_map_buffers(const scoped_file_descriptor& fd,
                       uint32_t allocated_buffers)
    {
        assert(fd);
        assert(allocated_buffers > 0);

        std::error_code error;
        auto buffers = memory_map_buffers(fd, allocated_buffers, error);
        throw_if_error(error);
        return buffers;
    }
}
}
