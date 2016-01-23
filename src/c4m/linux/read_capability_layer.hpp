// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>
#include <linux/videodev2.h>

namespace c4m
{
namespace linux
{
    /// Checks the capabilities of the device
    ///
    /// Docs: https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html
    ///
    template<class Super>
    class read_capability_layer2 : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            Super::open(device, error);

            if(error)
                return;

            memset(&m_capability, 0, sizeof(m_capability));

            Super::retry_ioctl(VIDIOC_QUERYCAP, &m_capability, error);
        }

        const v4l2_capability& capability() const
        {
            return m_capability;
        }

        bool is_a_video_capture_device() const
        {
            return m_capability.capabilities & V4L2_CAP_VIDEO_CAPTURE;
        }

        bool has_streaming_io_ioctls() const
        {
            return m_capability.capabilities & V4L2_CAP_STREAMING;
        }

    private:

        v4l2_capability m_capability;

    };
}
}
