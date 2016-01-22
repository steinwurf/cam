// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "retry_ioctl.hpp"

namespace c4m
{
namespace linux
{
    /// Requests the v4l2 driver to allocate a specific number of memory
    /// buffers.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    std::error_code error;
    ///    c4m::linux::request_memory_map_buff(fd, &format, error);
    ///
    ///    if(error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    /// Docs: https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-reqbufs.html
    ///
    inline uint32_t request_memory_map_buffers(const scoped_file_descriptor& fd,
                                               uint32_t requested_buffers,
                                               std::error_code& error)
    {
        assert(fd);
        assert(requested_buffers > 0);
        assert(error);

        struct v4l2_requestbuffers request = {0};

        request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        request.memory = V4L2_MEMORY_MMAP;
        request.count = requested_buffers;

        retry_ioctl(fd, VIDIOC_REQBUFS, &request, error);

        return request.count;
    }

    /// Calls read_format(...) with an error_code and throws an exception
    /// if an error is set.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    v4l2_format format;
    ///
    ///    c4m::linux::read_format(fd, &format);
    ///
    inline uint32_t request_memory_map_buffers(const scoped_file_descriptor& fd,
                                               uint32_t requested_buffers)
    {
        assert(fd);
        assert(requested_buffers);

        std::error_code error;
        uint32_t c = request_memory_map_buffers(fd, requested_buffers, error);
        throw_if_error(error);
        return c;
    }
}
}
