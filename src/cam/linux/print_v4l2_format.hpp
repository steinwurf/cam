// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "v4l2_pixelformat_to_string.hpp"

namespace cam
{
namespace linux
{
    /// Prints the v4l2_format struct
    ///
    /// Example:
    ///
    ///    v4l2_format format;
    ///
    ///    cam::linux::print_v4l2_format(std::cout, format);
    ///
    /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-fmt.html
    ///
    inline void print_v4l2_format(std::ostream& os, v4l2_format& format)
    {
        os << "Format:\n";
        os << "    Type: " << format.type << "\n";
        os << "    Width: " << format.fmt.pix.width << "\n";
        os << "    Height: " << format.fmt.pix.height << "\n";
        os << "    Codec: "
           << v4l2_pixelformat_to_string(format.fmt.pix.pixelformat) << "\n";
    }
}
}
