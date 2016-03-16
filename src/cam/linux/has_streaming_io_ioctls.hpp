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
    /// Checks whether the device supports the streaming io ioctls.
    ///
    /// Example:
    ///
    ///    // Assuming capability is a v4l2_capability struct initialized
    ///    // by a call to cam::linux::read_capability(...)
    ///
    ///    bool ok = cam::linux::has_streaming_io_ioctls(capability);
    ///
    /// Docs: https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html
    ///
    inline bool has_streaming_io_ioctls(const v4l2_capability& capability)
    {
        return capability.capabilities & V4L2_CAP_STREAMING;
    }
}
}
