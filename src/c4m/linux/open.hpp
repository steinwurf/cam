// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "scoped_file_descriptor.hpp"

#include "../throw_if_error.hpp"

#include <fcntl.h>

namespace c4m
{
namespace linux
{
    /// Opens the webcam device that we will be using or sets the error_code
    ///
    /// Example:
    ///
    ///    std::error_code error;
    ///    auto fd = c4m::linux::open("/dev/video1", error);
    ///
    ///    if(error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    ///    // Device is open
    ///
    /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/func-open.html
    ///
    inline scoped_file_descriptor open(const char* device,
                                       std::error_code& error)
    {
        assert(device);

        scoped_file_descriptor fd(::open(device, O_RDWR));

        // @todo Perhaps we don't need to assign 0 on success and the first
        //       part of the if can be removed
        if (fd)
        {
            error.assign(0, std::generic_category());
        }
        else
        {
            error.assign(errno, std::generic_category());
        }

        return fd;
    }

    /// Calls open(...) with an error_code and throws an exception
    /// if an error is set.
    ///
    /// Example:
    ///
    ///    auto fd = c4m::linux::open("/dev/video1");
    ///
    ///    // Device is open
    ///
    inline scoped_file_descriptor open(const char* device)
    {
        std::error_code error;
        auto fd = open(device, error);
        throw_if_error(error);
        return fd;
    }
}
}
