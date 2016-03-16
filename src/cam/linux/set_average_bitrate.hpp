// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "uvcx_bitrate.hpp"

#include <linux/usb/video.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <system_error>

namespace cam
{
namespace linux
{
    template<class Super>
    class set_average_bitrate : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if (error)
                return;

            memset(&m_bitrates, 0, sizeof(m_bitrates));
            Super::query(0x0E, UVC_GET_CUR, (uint8_t*) &m_bitrates, error);
        }

        void request_bitrates(uint32_t average_bitrate, uint32_t peak_bitrate,
                              std::error_code& error)
        {
            assert(!error);
            assert(Super::is_status_open() || Super::is_status_streaming());

            m_bitrates.m_peak_bitrate = peak_bitrate;
            m_bitrates.m_average_bitrate = average_bitrate;

            Super::query(0x0E, UVC_SET_CUR, (uint8_t*) &m_bitrates, error);
        }

        uint32_t average_bitrate() const
        {
            return m_bitrates.m_average_bitrate;
        }

        uint32_t peak_bitrate() const
        {
            return m_bitrates.m_peak_bitrate;
        }

    private:

        uvcx_bitrate m_bitrates;
    };
}
}
