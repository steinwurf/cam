// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "retry_ioctl.hpp"

namespace c4m
{
namespace linux
{
    /// Prints the v4l2_format struct
    ///
    /// Example:
    ///
    ///    v4l2_format format;
    ///
    ///    c4m::linux::print_v4l2_format(std::cout, format);
    ///
    /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-fmt.html
    ///
    inline void print_v4l2_format(std::ostream& os, v4l2_format& format)
    {
        os << "Format:\n";
        os << "    Type: " << format.type << "\n";
        os << "    Width: " << format.fmt.pix.width << "\n";
        os << "    Height: " << format.fmt.pix.height << "\n";

        uint32_t pixelformat = format.fmt.pix.pixelformat;

        char fourcc[] = {(char) (pixelformat & 0xff),
                         (char) ((pixelformat >> 8) & 0xff),
                         (char) ((pixelformat >> 16) & 0xff),
                         (char) ((pixelformat >> 24) & 0xff)};

        os << "    Codec: " << std::string(fourcc, 4) << "\n";
    }
}
}
