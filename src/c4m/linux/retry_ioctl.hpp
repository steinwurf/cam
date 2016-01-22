// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "scoped_file_descriptor.hpp"
#include "ioctl.hpp"

namespace c4m
{
namespace linux
{
    /// Retries the ioctl as long as it fails with the error code EINT,
    /// which means that the request for some reason was interrupted. If
    /// the ioctl(...) succeds or fails with some other error code we exit
    /// the retry loop.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    std::error_code error;
    ///    c4m::linux::retry_ioctl(fd, some_request, &some_data, &error);
    ///
    ///    if(error)
    ///    {
    ///        // Something went wrong
    ///    }
    ///
    inline void retry_ioctl(const scoped_file_descriptor& fd, int request,
                            void *arg, std::error_code& error)
    {
        assert(fd);

        do
        {
            ioctl(fd, request, arg, error);
        }
        while (error == std::errc::interrupted);
    }

    /// Calls retry_ioctl(...) with an error_code and throws an exception
    /// if an error is set.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    c4m::linux::retry_ioctl(fd, some_request, &some_data);
    ///
    inline void retry_ioctl(const scoped_file_descriptor& fd, int request,
                            void *arg)
    {
        assert(fd);

        std::error_code error;
        retry_ioctl(fd, request, arg, error);
        throw_if_error(error);
    }
}
}
