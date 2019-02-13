// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>
#include <sstream>

namespace cam
{
namespace linux
{
    template<class Super>
    class trace_set_average_bitrate : public Super
    {
    public:

        void open(const std::string& device, std::error_code& error)
        {
            assert(!device.empty());
            assert(!error);

            Super::open(device, error);

            if (Super::is_trace_enabled())
            {
                if (error)
                {
                    Super::write_trace(
                        "set_average_bitrate", "open failed");
                }
                else
                {
                    Super::write_trace(
                        "set_average_bitrate", "open success");
                }
            }
        }

        void request_bitrates(uint32_t average_bitrate, uint32_t peak_bitrate,
                              std::error_code& error)
        {
            Super::request_bitrates(average_bitrate, peak_bitrate, error);
        }

    };
}
}
