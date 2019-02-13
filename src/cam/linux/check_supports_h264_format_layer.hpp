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
    class check_supports_h264_format_layer : public Super
    {
    public:

        void open(const std::string& device, std::error_code& error)
        {
            Super::open(device, error);

            if (error)
            {
                return;
            }

            if (!Super::supports_pixelformat("H264"))
            {
                error = cam::error::does_not_support_h264_codec;
                return;
            }
        }
    };
}
}
