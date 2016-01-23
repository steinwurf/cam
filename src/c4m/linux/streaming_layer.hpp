// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <vector>

namespace c4m
{
namespace linux
{
    template<class Super>
    class streaming_layer : public Super
    {
    public:

        /// Start streaming
        ///
        /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-streamon.html
        ///
        void start_streaming(std::error_code& error)
        {
            assert(!error);

            Super::start_streaming(error);

            if (error)
            {
                return;
            }

            uint32_t type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            Super::retry_ioctl(VIDIOC_STREAMON, &type, error);
        }

        /// Stop streaming
        ///
        /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-streamon.html
        ///
        void stop_streaming(std::error_code& error)
        {
            assert(!error);

            uint32_t type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            Super::retry_ioctl(VIDIOC_STREAMOFF, &type, error);

            if (!error)
            {
                Super::stop_streaming(error);
            }
        }

    };
}
}
