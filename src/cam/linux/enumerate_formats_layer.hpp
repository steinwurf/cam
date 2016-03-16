// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "string_to_v4l2_pixelformat.hpp"

#include <linux/videodev2.h>

#include <system_error>
#include <vector>

namespace cam
{
namespace linux
{
    template<class Super>
    class enumerate_formats_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if(error)
            {
                return;
            }

            // There are more supported  buffer types for this operation:
            // http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-fmt.html
            //
            // But we are only interested in video capture and we do not
            // support multi-planar buffers
            v4l2_fmtdesc format;
            memset(&format, 0, sizeof(format));
            format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            std::error_code query_error;

            for (format.index = 0;; ++format.index)
            {
                Super::retry_ioctl(VIDIOC_ENUM_FMT, &format, query_error);

                if (query_error)
                {
                    break;
                }

                m_formats.push_back(format);
            }

            // Is not actually an error to get an invalid argument error
            // here - but the condition when there are no more formats to
            // enumerate. So we only set an error if we get something different.
            if (query_error != std::errc::invalid_argument)
            {
                error = query_error;
            }
        }

        bool supports_pixelformat(const std::string& fourcc)
        {
            assert(fourcc.size() == 4U);

            uint32_t pixelformat = string_to_v4l2_pixelformat(fourcc);

            for(const auto& format : m_formats)
            {
                if (format.pixelformat == pixelformat)
                    return true;
            }

            return false;
        }

        const std::vector<v4l2_fmtdesc>& formats() const
        {
            return m_formats;
        }

    private:

        std::vector<v4l2_fmtdesc> m_formats;

    };
}
}
