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
    /// Sets the current format.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    std::error_code error;
    ///    v4l2_format format;
    ///
    ///    c4m::linux::set_format(fd, &format, error);
    ///
    ///    if(error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-fmt.html
    ///
    inline void set_format(const scoped_file_descriptor& fd,
                           v4l2_format* format, std::error_code& error)
    {
        assert(fd);
        assert(format);

        retry_ioctl(fd, VIDIOC_S_FMT, format, error);
    }

    /// Calls set_format(...) with an error_code and throws an exception
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
    inline void set_format(const scoped_file_descriptor& fd,
                           v4l2_format* format)
    {
        assert(fd);
        assert(format);

        std::error_code error;
        set_format(fd, format, error);
        throw_if_error(error);
    }
}
}
