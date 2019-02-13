// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "../xu_h264_codec_control.hpp"
#include "../throw_if_error.hpp"

#include "uvcx_video_config.hpp"
#include "uvcx_bitrate.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <system_error>

namespace cam
{
namespace linux
{
template<class Camera>
class xu_h264_codec_control_impl : public xu_h264_codec_control
{
    /// The following is defined in:
    ///    USB Device Class Definition for Video Devices: H.264 Payload.
    ///    Revision: 1.00
    ///    Section "3.3 H.264 UVC Extensions Units (XUs)" (p. 11)
    ///
    enum class selector
    {
        uvcx_video_config_probe = 0x01,
        uvcx_video_config_commit = 0x02,
        uvcx_bitrate_layers = 0xE
    };

public:

    xu_h264_codec_control_impl(Camera& camera, uint8_t unit_id) :
        m_camera(camera), m_unit_id(unit_id)
    { }

    void open(const std::string& device, std::error_code& error)
    {
        assert(!device.empty());
        assert(!error);

        memset(&m_config, 0, sizeof(m_config));
        memset(&m_bitrates, 0, sizeof(m_bitrates));

        m_camera.xu_query(
            m_unit_id,
            static_cast<uint8_t>(selector::uvcx_video_config_probe),
            UVC_GET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);

        if (error)
            return;

        m_camera.xu_query(
            m_unit_id,
            static_cast<uint8_t>(selector::uvcx_bitrate_layers),
            UVC_GET_CUR, reinterpret_cast<uint8_t*>(&m_bitrates), error);
    }

    void start_streaming(std::error_code& error)
    {
        assert(!error);

        // No buffers should be allocated yet, we need to configure the
        // encoder before we map memory etc. It is not a fact, but
        // seems reasonable since the size of memory will depend on the
        // chosen resolution etc.
        assert(m_camera.buffer_count() == 0);

        m_camera.xu_query(m_unit_id,
                          static_cast<uint8_t>(selector::uvcx_video_config_commit),
                          UVC_SET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);
    }

    uint32_t i_frame_period() const override
    {
        return m_config.m_i_frame_period;
    }

    void try_request_i_frame_period(uint32_t i_frame_period) override
    {
        std::error_code error;
        request_i_frame_period(i_frame_period, error);

        throw_if_error(error);
    }

    void request_i_frame_period(uint32_t i_frame_period,
                                std::error_code& error) override
    {
        assert(!error);

        m_config.m_i_frame_period = i_frame_period;

        m_camera.xu_query(
            m_unit_id,
            static_cast<uint8_t>(selector::uvcx_video_config_probe),
            UVC_SET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);
    }

    void try_request_bitrates(uint32_t average_bitrate,
                              uint32_t peak_bitrate) override
    {
        std::error_code error;
        request_bitrates(average_bitrate, peak_bitrate, error);

        throw_if_error(error);
    }

    void request_bitrates(uint32_t average_bitrate, uint32_t peak_bitrate,
                          std::error_code& error) override
    {
        assert(!error);
        assert(m_camera.is_status_open() || m_camera.is_status_streaming());

        m_bitrates.m_peak_bitrate = peak_bitrate;
        m_bitrates.m_average_bitrate = average_bitrate;

        m_camera.xu_query(
            m_unit_id,
            static_cast<uint8_t>(selector::uvcx_bitrate_layers),
            UVC_SET_CUR, reinterpret_cast<uint8_t*>(&m_bitrates), error);
    }

    uint32_t average_bitrate() const override
    {
        return m_bitrates.m_average_bitrate;
    }

    uint32_t peak_bitrate() const override
    {
        return m_bitrates.m_peak_bitrate;
    }



    Camera& m_camera;
    uint8_t m_unit_id;

    uvcx_video_config m_config;
    uvcx_bitrate m_bitrates;
};
}
}
