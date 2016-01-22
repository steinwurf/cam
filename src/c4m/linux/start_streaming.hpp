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
    /// Start streaming
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    std::error_code error;
    ///    auto buffers = c4m::linux::start_streaming(fd, error);
    ///
    ///    if (error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-streamon.html
    ///
    inline void start_streaming(const scoped_file_descriptor& fd,
                                std::error_code& error)
    {
        assert(fd);

        uint32_t type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        retry_ioctl(fd, VIDIOC_STREAMON, &type, error);
    }

    /// Calls start_streaming(...) with an error_code and throws an
    /// exception if an error is set.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    c4m::linux::start_streaming(fd);
    ///
    inline void start_streaming(const scoped_file_descriptor& fd)
    {
        assert(fd);

        std::error_code error;
        start_streaming(fd, error);
        throw_if_error(error);
    }
}
}
