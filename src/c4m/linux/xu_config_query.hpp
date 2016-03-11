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

namespace c4m
{
namespace linux
{
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

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if (error)
                return;

            memset(&m_config, 0, sizeof(m_config));

            Super::query(
                static_cast<uint8_t>(selector::uvcx_video_config_probe),
                UVC_GET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);
        }


        void start_streaming(std::error_code& error)
        {
            assert(!error);

            // No buffers should be allocated yet, we need to configure the
            // encoder before we map memory etc. It is not a fact, but
            // seems reasonable since the size of memory will depend on the
            // chosen resolution etc.
            assert(Super::buffer_count() == 0);

            Super::query(
                static_cast<uint8_t>(selector::uvcx_video_config_commit),
                UVC_SET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);

            if (error)
                return;

            Super::start_streaming(error);
        }

        // uint32_t width() const
        // {
        //     return m_config.m_width;
        // }

        // uint32_t height() const
        // {
        //     return m_config.m_height;
        // }

        // void request_resolution(uint32_t width, uint32_t height,
        //                         std::error_code& error)
        // {
        //     assert(!error);
        //     assert(width > 0);
        //     assert(height > 0);

        //     std::cout << "request res" << std::endl;

        //     m_config.m_width = width;
        //     m_config.m_height = height;

        //     Super::query(
        //         static_cast<uint8_t>(selector::uvcx_video_config_probe),
        //         UVC_SET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);
        // }

        uint32_t i_frame_period() const
        {
            return m_config.m_i_frame_period;
        }

        void request_i_frame_period(uint32_t i_frame_period,
                                    std::error_code& error)
        {
            assert(!error);

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
