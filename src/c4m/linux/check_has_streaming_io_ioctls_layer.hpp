// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "../error.hpp"
#include "../is_error_code_enum.hpp"
#include "../error_category.hpp"
#include "../make_error_code.hpp"

namespace c4m
{
namespace linux
{
    template<class Super>
    class check_has_streaming_io_ioctl_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            Super::open(device, error);

            if (error)
                return;

            if (!Super::has_streaming_io_ioctls())
            {
                error = c4m::error::does_not_have_streaming_io_ioctls;
                return;
            }
        }
    };
}
}
