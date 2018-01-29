// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>
#include <sstream>

namespace cam
{
namespace linux
{
    /// Passes an ioctl request to the webcam device that we are using or
    /// sets the error_code.
    template<class Super>
    class trace_ioctl_layer : public Super
    {
    public:

        void ioctl(uint64_t request, void* arg, std::error_code& error)
        {
            Super::ioctl(request, arg, error);

            if (Super::is_trace_enabled())
            {

                std::stringstream ss;
                ss << "request=" << request << " ";
                ss << "arg=" << arg << " ";
                ss << "error=" << error << " ";

                if (error)
                    ss << "error_message=" << error.message();

                Super::write_trace(
                    "ioctl", ss.str());
            }
        }
    };
}
}
