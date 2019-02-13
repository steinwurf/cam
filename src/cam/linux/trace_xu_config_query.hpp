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
    class trace_xu_config_query : public Super
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
                        "xu_config_query", "open failed");
                }
                else
                {
                    Super::write_trace(
                        "xu_config_query", "open success");
                }
            }
        }


        void start_streaming(std::error_code& error)
        {
            Super::start_streaming(error);
        }


        void request_i_frame_period(uint32_t i_frame_period,
                                    std::error_code& error)
        {
            Super::request_i_frame_period(i_frame_period, error);
        }

    };
}
}
