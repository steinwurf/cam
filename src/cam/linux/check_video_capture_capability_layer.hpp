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

namespace cam
{
namespace linux
{
    template<class Super>
    class check_video_capture_capability_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            Super::open(device, error);

            if(error)
            {
                return;
            }

            if (!Super::is_a_video_capture_device())
            {
                error = cam::error::not_a_video_capture_device;
                return;
            }
        }
    };
}
}
