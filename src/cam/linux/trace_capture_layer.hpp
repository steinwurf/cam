// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <vector>
#include <sstream>

#include <n4lu/to_annex_b_nalus.hpp>

#include "../trace_layer.hpp"
#include "v4l2_timestamp_to_micro_seconds.hpp"

namespace cam
{
namespace linux
{
    /// Fall-through case for the case where TraceTag is meta::not_found
    template<class TraceTag, class Super>
    class trace_capture_layer : public Super
    {
        static_assert(std::is_same<TraceTag, meta::not_found>::value,
                      "Unexpected TraceTag should be meta::not_found in the "
                      "fall-through case.");
    };

    /// Trace the data capture
    template<class Super>
    class trace_capture_layer<enable_trace, Super> : public Super
    {
    public:

        capture_data capture(std::error_code& error)
        {
            assert(!error);

            auto data = Super::capture(error);

            if (Super::is_trace_enabled())
            {
                Super::write_trace(
                    "capture_layer", to_string_capture_data(data));
            }

            return data;
        }

    private:

        std::string to_string_capture_data(const capture_data& data)
        {
            std::stringstream ss;

            ss << data << "\n";

            auto nalus = n4lu::to_annex_b_nalus(data.m_data, data.m_size);

            for(const auto& nalu : nalus)
            {
                ss << "  " << nalu << "\n";
                assert(nalu);
            }

            return ss.str();

        }
    };
}
}
