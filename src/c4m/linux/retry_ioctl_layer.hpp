// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>
#include <cstdint>

namespace c4m
{
namespace linux
{
    /// Retries the ioctl as long as it fails with the error code EINT,
    /// which means that the request for some reason was interrupted. If
    /// the ioctl(...) succeds or fails with some other error code we exit
    /// the retry loop.
    template<class Super>
    class retry_ioctl_layer : public Super
    {
    public:

        void retry_ioctl(uint64_t request, void *arg, std::error_code& error)
        {
            assert(!error);

            do
            {
                Super::ioctl(request, arg, error);
            }
            while (error == std::errc::interrupted);

        }
    };
}
}
