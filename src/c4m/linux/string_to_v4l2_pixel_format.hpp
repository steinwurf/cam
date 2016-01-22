// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <linux/videodev2.h>

namespace c4m
{
namespace linux
{
    /// Converts the v4l2 pixelformat to a string
    ///
    /// Docs: https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-fmt.html
    ///
    uint32_t string_to_v4l2_pixel_format(const std::string& fourcc)
    {
        assert(fourcc.size() == 4U);
        return v4l2_fourcc(fourcc[0],fourcc[1],fourcc[2],fourcc[3]);
    }
}
}
