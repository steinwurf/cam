// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <linux/videodev2.h>

namespace cam
{
namespace linux
{
    /// Converts the v4l2 pixelformat to a string
    ///
    /// Docs: https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-fmt.html
    ///
    inline std::string v4l2_pixelformat_to_string(uint32_t pixelformat)
    {
        char fourcc[] = {(char) (pixelformat & 0xff),
                         (char) ((pixelformat >> 8) & 0xff),
                         (char) ((pixelformat >> 16) & 0xff),
                         (char) ((pixelformat >> 24) & 0xff)};

        return std::string(fourcc, 4);
    }
}
}
