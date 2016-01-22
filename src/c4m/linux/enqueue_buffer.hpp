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
    /// Enqueues one of the allocated buffers.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    std::error_code error;
    ///    auto buffers = c4m::linux::enqueue_buffer(fd, 4, error);
    ///
    ///    if (error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-qbuf.html
    ///
    inline void enqueue_buffer(const scoped_file_descriptor& fd, uint32_t index,
                               std::error_code& error)
    {
        assert(fd);

        v4l2_buffer buffer;
        memset(&buffer, 0, sizeof(buffer));

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = index;

        retry_ioctl(fd, VIDIOC_QBUF, &buffer, error);
    }

    /// Calls enqueue_buffer(...) with an error_code and throws an
    /// exception if an error is set.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    c4m::linux::enqueue_buffer(fd, 4);
    ///
    inline void enqueue_buffer(const scoped_file_descriptor& fd, uint32_t index)
    {
        assert(fd);

        std::error_code error;
        enqueue_buffer(fd, index, error);
        throw_if_error(error);
    }
}
}
