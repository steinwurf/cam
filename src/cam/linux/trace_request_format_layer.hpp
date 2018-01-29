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
    class trace_request_format_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if (Super::is_trace_enabled())
            {
                if (error)
                {
                    Super::write_trace(
                        "request_format_layer", "open failed");
                }
                else
                {
                    Super::write_trace(
                        "request_format_layer", "open success");
                }
            }
        }

        void request_resolution(uint32_t width, uint32_t height,
                                std::error_code& error)
        {
            Super::request_resolution(width, height, error);
        }

        void request_pixelformat(const std::string& fourcc,
                                 std::error_code& error)
        {
            Super::request_pixelformat(fourcc, error);
        }

    };

}
}
