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
    uint64_t v4l2_timestamp_to_micro_seconds(const v4l2_buffer& buffer)
    {

        auto clock_type = buffer.flags & V4L2_BUF_FLAG_TIMESTAMP_MASK;
        assert(clock_type == V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC);

        uint64_t time = (buffer.timestamp.tv_sec * 1000000) +
            buffer.timestamp.tv_usec;

        // @todo if clock type is V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN
        // we can do the timestamp ourselves with some chrono or such

        return time;
    }
}
}
