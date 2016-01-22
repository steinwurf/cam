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
    /// Checks whether the device is a video capture device (e.g. webcam)
    ///
    /// Example:
    ///
    ///    // Assuming capability is a v4l2_capability struct initialized
    ///    // by a call to c4m::linux::read_capability(...)
    ///
    ///    bool ok = c4m::linux::is_a_video_capture_device(capability);
    ///
    /// Docs: https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html
    ///
    inline bool is_a_video_capture_device(const v4l2_capability& capability)
    {
        return capability.capabilities & V4L2_CAP_VIDEO_CAPTURE;
    }
}
}
