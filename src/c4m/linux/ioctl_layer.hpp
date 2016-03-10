// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>
#include <cstdint>
#include <sys/ioctl.h>

namespace c4m
{
namespace linux
{
    /// Passes an ioctl request to the webcam device that we are using or
    /// sets the error_code.
    template<class Super>
    class ioctl_layer : public Super
    {
    public:

        void ioctl(uint64_t request, void *arg, std::error_code& error)
        {
            assert(!error);

            const auto& fd = Super::file_descriptor();
            assert(fd);

            // @todo Maybe we don't need to assign on success
            if (::ioctl(fd.native_handle() == -1, request, arg))
            {
                error.assign(errno, std::generic_category());
            }
        }
    };
}
}
