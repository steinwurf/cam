// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "scoped_file_descriptor.hpp"

#include <sys/ioctl.h>

namespace c4m
{
namespace linux
{
    /// Passes an ioctl request to the webcam device that we are using or
    /// sets the error_code.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    std::error_code error;
    ///    c4m::linux::ioctl(fd, some_request, &some_data, &error);
    ///
    ///    if(error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/func-ioctl.html
    ///
    inline void ioctl(const scoped_file_descriptor& fd, int request, void *arg,
               std::error_code& error)
    {
        assert(fd);

        // @todo Maybe we don't need to assign on success
        if (-1 == ::ioctl(fd.native_handle(), request, arg))
        {
            error.assign(errno, std::generic_category());
        }
        else
        {
            error.assign(0, std::generic_category());
        }
    }
}
}
