// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <sstream>

namespace cam
{
namespace linux
{
    /// Trace opening the file
    template<class Super>
    class trace_open_layer : public Super
    {
    public:

        void open(const std::string& device, std::error_code& error)
        {
            if (Super::is_trace_enabled())
            {
                std::stringstream ss;
                ss << "open " << device;
                Super::write_trace(
                    "open_layer", ss.str());
            }

            Super::open(device, error);

            if (Super::is_trace_enabled())
            {
                if (error)
                {
                    Super::write_trace(
                        "open_layer", "open failed");
                }
                else
                {
                    Super::write_trace(
                        "open_layer", "open success");
                }
            }
        }
    };
}
}
