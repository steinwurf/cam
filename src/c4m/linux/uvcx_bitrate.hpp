// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>

namespace c4m
{
namespace linux
{
    /// The following is defined in:
    ///     USB Device Class Definition for Video Devices: H.264 Payload.
    ///     Revision: 1.00
    ///     Section "3.3.14 UVCX_BITRATE_LAYERS" (p. 35)
    struct uvcx_bitrate
    {
        /// Bitmask for a number of IDs, for a single-stream and
        /// single-layer H.264 stream this should always be zero (see
        /// section 3.3.2.1).
        uint16_t m_layer_id;

        /// Peak bit-rate in bits/sec for the specific layer
        uint32_t m_peak_bitrate;

        /// Average bit-rate in bits/sec for the specific layer
        uint32_t m_average_bitrate;

    } __attribute__((packed));

    // Output operator for the uvcx_video_config
    inline std::ostream&
    operator<<(std::ostream& os, const uvcx_bitrate& bitrates)
    {
        os << "c4m::linux::uvcx_bitrate: "
           << "m_layer_id = " << bitrates.m_layer_id << " "
           << "m_peak_bitrate = " << bitrates.m_peak_bitrate << " "
           << "m_average_bitrate = " << bitrates.m_average_bitrate << " ";

        return os;
    }
}
}
