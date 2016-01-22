// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "scoped_file_descriptor.hpp"
#include "retry_ioctl.hpp"

#include <linux/videodev2.h>

namespace c4m
{
namespace linux
{
    /// Query the device capabilities
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    v4l2_capability capability;
    ///    std::error_code error;
    ///
    ///    c4m::linux::read_capability(fd, &capability, error);
    ///
    ///    if(error)
    ///    {
    ///        // Something went wrong
    ///    }
    ///
    /// Docs: https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html
    ///
    inline void read_capability(const scoped_file_descriptor& fd,
                                v4l2_capability* capability,
                                std::error_code& error)
    {
        assert(fd);
        assert(capability);

        retry_ioctl(fd, VIDIOC_QUERYCAP, capability, error);
    }

    /// Calls read_cabability(...) with an error_code and throws an exception
    /// if an error is set.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    v4l2_capability capability;
    ///
    ///    c4m::linux::read_capability(fd, &capability);
    ///
    inline void read_capability(const scoped_file_descriptor& fd,
                                v4l2_capability* capability)
    {
        assert(fd);
        assert(capability);

        std::error_code error;
        read_capability(fd, capability, error);
        throw_if_error(error);
    }
}
}
