// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <ostream>

namespace cam
{
namespace linux
{
    /// UVCX Video Config struct is used for probing and to config a
    /// specific configuration
    ///
    /// Defined in:
    ///    USB Device Class Definition for Video Devices: H.264 Payload.
    ///    Revision: 1.00
    ///    Page 11.
    ///    Link:
    ///
    struct uvcx_video_config
    {
        uint32_t m_frame_interval;
        uint32_t m_bitrate;
        uint16_t m_hints;
        uint16_t m_configuration_index;
        uint16_t m_width;
        uint16_t m_height;
        uint16_t m_slice_units;
        uint16_t m_slice_mode;
        uint16_t m_profile;
        uint16_t m_i_frame_period;
        uint16_t m_estimated_video_delay;
        uint16_t m_estimated_max_config_delay;
        uint8_t m_usage_type;
        uint8_t m_rate_control_mode;
        uint8_t m_temporal_scale_mode;
        uint8_t m_spatial_scale_mode;
        uint8_t m_snr_scale_mode;
        uint8_t m_stream_mux_option;
        uint8_t m_stream_format;
        uint8_t m_entropy_cabac;
        uint8_t m_timestamp;
        uint8_t m_num_of_reorder_frames;
        uint8_t m_preview_flipped;
        uint8_t m_view;
        uint8_t m_reserved1;
        uint8_t m_reserved2;
        uint8_t m_stream_id;
        uint8_t m_spatial_layer_ratio;
        uint16_t m_leaky_bucket_size;
    } __attribute__((packed));

    // Output operator for the uvcx_video_config
    inline std::ostream&
    operator<<(std::ostream& os, const uvcx_video_config& config)
    {
        os << "cam::linux::uvcx_video_config: "
           << "m_frame_interval = " << config.m_frame_interval << " "
           << "m_bitrate = " << config.m_bitrate << " "
           << "m_hints = " << config.m_hints << " "
           << "m_configuration_index = " << config.m_configuration_index << " "
           << "m_width = " << config.m_width << " "
           << "m_height = " << config.m_height << " "
           << "m_slice_units = " << config.m_slice_units << " "
           << "m_slice_mode = " << config.m_slice_mode << " "
           << "m_profile = " << config.m_profile << " "
           << "m_i_frame_period = " << config.m_i_frame_period << " "
           << "m_estimated_video_delay = "
           << config.m_estimated_video_delay << " "
           << "m_estimated_max_config_delay = "
           << config.m_estimated_max_config_delay << " "
           << "m_usage_type = " << config.m_usage_type << " ";

        return os;
    }
}
}
