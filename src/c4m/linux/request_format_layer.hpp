// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>
#include <linux/videodev2.h>

#include "v4l2_pixelformat_to_string.hpp"

namespace c4m
{
namespace linux
{
    template<class Super>
    class request_format_layer2 : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if (error)
                return;

            memset(&m_format, 0, sizeof(m_format));

            // For single-planar API
            m_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            Super::retry_ioctl(VIDIOC_G_FMT, &m_format, error);
        }

        void request_resolution(uint32_t width, uint32_t height,
                                std::error_code& error)
        {
            assert(width > 0);
            assert(height > 0);
            assert(!error);

            m_format.fmt.pix.width = width;
            m_format.fmt.pix.height = height;

            Super::retry_ioctl(VIDIOC_S_FMT, &m_format, error);
        }

        void request_pixelformat(const std::string& fourcc,
                                 std::error_code& error)
        {
            assert(fourcc.size() == 4);
            assert(Super::supports_pixelformat(fourcc));
            assert(!error);

            m_format.fmt.pix.pixelformat = string_to_v4l2_pixelformat(fourcc);

            Super::retry_ioctl(VIDIOC_S_FMT, &m_format, error);
        }

        uint32_t width() const
        {
            return m_format.fmt.pix.width;
        }

        uint32_t height() const
        {
            return m_format.fmt.pix.height;
        }

        std::string pixelformat() const
        {
            return v4l2_pixelformat_to_string(m_format.fmt.pix.pixelformat);
        }

    private:

        v4l2_format m_format;
    };

}
}
