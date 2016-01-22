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

    /// Dequeue a buffer with video data
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    auto buffer = c4m::linux::dequeue_buffer(fd, error);
    ///
    ///    if (error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-qbuf.html
    ///
    inline v4l2_buffer dequeue_buffer(const scoped_file_descriptor& fd,
                                      std::error_code& error)
    {
        assert(fd);

        v4l2_buffer buffer;
        memset(&buffer, 0, sizeof(buffer));

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;

        retry_ioctl(fd, VIDIOC_DQBUF, &buffer, error);

        return buffer;
    }

    /// Calls dequeue_buffer(...) with and error_code and throws and
    /// exception if an error is set.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    auto buffer = c4m::linux::dequeue_buffer(fd);
    ///
    inline v4l2_buffer dequeue_buffer(const scoped_file_descriptor& fd)
    {
        assert(fd);

        std::error_code error;
        auto buffer = dequeue_buffer(fd, error);
        throw_if_error(error);
        return buffer;
    }
}
}
