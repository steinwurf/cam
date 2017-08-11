// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "../error.hpp"

namespace cam
{
namespace linux
{
    template<class Super>
    class set_h264_format_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if (error)
                return;

            Super::request_pixelformat("H264", error);

            if (error)
                return;

            if (Super::pixelformat() != "H264")
                error = cam::error::format_is_not_h264;
        }


        /// We don't want anybody to override our decision here so lets
        /// delete the possiblity to call request_pixelformat(...) from
        /// outside the stack
        void request_pixelformat(const std::string& fourcc,
                                 std::error_code& error) = delete;
    };
}
}
