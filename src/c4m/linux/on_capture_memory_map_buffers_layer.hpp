// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>

#include "capture_data.hpp"

namespace c4m
{
namespace linux
{
    /// Will ensure that buffers are memory mapped when the user calls capture
    template<class Super>
    class on_capture_memory_map_buffers_layer : public Super
    {
    public:

        /// Maps buffers if not already done
        ///
        /// @param error Error code will be set if an error occurs.
        /// @return The captured data
        capture_data capture(std::error_code& error)
        {
            assert(Super::is_state_streaming());

            if (!Super::is_memory_map_complete())
            {
                Super::memory_map_buffers(error);

                if (error)
                {
                    return capture_data();
                }
            }

            return Super::capture_data(error);
        }
    };
}
}
