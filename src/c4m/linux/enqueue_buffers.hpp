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
    /// Enqueues a number of allocated buffers.
    ///
    /// Example:
    ///
    ///    std::error_code error;
    ///    c4m::linux::enqueue_buffers(fd, 10, error);
    ///
    ///    if (error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-qbuf.html
    ///
    inline void enqueue_buffers(const scoped_file_descriptor& fd,
                                uint32_t allocated_buffers,
                                std::error_code& error)
    {
        assert(fd);
        assert(allocated_buffers > 0);

        for (uint32_t i = 0; i < allocated_buffers; ++i)
        {
            enqueue_buffer(fd, i, error);

            if (error)
            {
                return;
            }
        }
    }

    /// Calls enqueue_buffers(...) with an error_code and throws an
    /// exception if an error is set.
    ///
    /// Example:
    ///
    ///    c4m::linux::enqueue_buffers(fd, 10);
    ///
    inline void enqueue_buffers(const scoped_file_descriptor& fd,
                                uint32_t allocated_buffers)
    {
        assert(fd);
        assert(allocated_buffers > 0);

        std::error_code error;
        enqueue_buffers(fd, allocated_buffers, error);
        throw_if_error(error);
    }
}
}
