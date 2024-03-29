// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "uvcx_video_config.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <system_error>

namespace cam
{
namespace linux
{

template<class Parent>
class h264
{
    Parent& Super;

};




template<class Super>
class xu_config_query : public Super
{
private:

    /// The following is defined in:
    ///    USB Device Class Definition for Video Devices: H.264 Payload.
    ///    Revision: 1.00
    ///    Section "3.3 H.264 UVC Extensions Units (XUs)" (p. 11)
    ///
    enum class selector
    {
        uvcx_video_config_probe = 0x01,
        uvcx_video_config_commit = 0x02
    };

public:

    void open(const std::string& device, std::error_code& error)
    {
        assert(!device.empty());
        assert(!error);

        Super::open(device, error);

        if (error)
            return;

        if (Super::has_h264_codec_control_unit_id())
        {

            memset(&m_config, 0, sizeof(m_config));

            Super::query(
                static_cast<uint8_t>(selector::uvcx_video_config_probe),
                UVC_GET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);
        }
    }


    void start_streaming(std::error_code& error)
    {
        assert(!error);

        // No buffers should be allocated yet, we need to configure the
        // encoder before we map memory etc. It is not a fact, but
        // seems reasonable since the size of memory will depend on the
        // chosen resolution etc.
        assert(Super::buffer_count() == 0);

        if (Super::has_h264_codec_control_unit_id())
        {
            Super::query(
                static_cast<uint8_t>(selector::uvcx_video_config_commit),
                UVC_SET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);
        }

        if (error)
            return;

        Super::start_streaming(error);
    }

    uint32_t i_frame_period() const
    {
        assert(Super::has_h264_codec_control_unit_id());
        return m_config.m_i_frame_period;
    }

    void request_i_frame_period(uint32_t i_frame_period,
                                std::error_code& error)
    {
        assert(!error);
        assert(Super::has_h264_codec_control_unit_id());

        m_config.m_i_frame_period = i_frame_period;

        Super::query(
            static_cast<uint8_t>(selector::uvcx_video_config_probe),
            UVC_SET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);
    }

private:

    uvcx_video_config m_config;
};
}
}
