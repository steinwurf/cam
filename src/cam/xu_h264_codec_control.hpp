// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cassert>
#include <cstdint>
#include <system_error>

namespace cam
{

class xu_h264_codec_control
{
public:

    virtual uint32_t i_frame_period() const = 0;

    virtual void try_request_i_frame_period(uint32_t i_frame_period) = 0;

    virtual void request_i_frame_period(uint32_t i_frame_period,
                                        std::error_code& error) = 0;

    virtual void try_request_bitrates(uint32_t average_bitrate,
                                      uint32_t peak_bitrate) = 0;

    virtual void request_bitrates(uint32_t average_bitrate,
                                  uint32_t peak_bitrate,
                                  std::error_code& error) = 0;

    virtual uint32_t average_bitrate() const = 0;

    virtual uint32_t peak_bitrate() const = 0;

};

}
