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
    /// Reads current format
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    std::error_code error;
    ///
    ///    auto format = c4m::linux::read_format(fd, error);
    ///
    ///    if(error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-fmt.html
    ///
    inline v4l2_format read_format(const scoped_file_descriptor& fd,
                                   std::error_code& error)
    {
        assert(fd);

        v4l2_format format = {0};

        // For single-planar APO
        format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        retry_ioctl(fd, VIDIOC_G_FMT, &format, error);
    }

    /// Calls read_format(...) with and error_code and throws and exception
    /// if an error is set.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    auto format = c4m::linux::read_format(fd);
    ///
    inline v4l2_format read_format(const scoped_file_descriptor& fd)
    {
        assert(fd);

        std::error_code error;
        auto format = read_format(fd, error);
        throw_if_error(error);
        return format;
    }
}
}
