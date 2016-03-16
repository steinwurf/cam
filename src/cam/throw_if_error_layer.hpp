// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "capture_data.hpp"
#include "throw_if_error.hpp"

namespace cam
{
    /// Layer which transforms the error_code based API to one that uses
    /// exceptions.
    template<class Super>
    class throw_if_error_layer : public Super
    {
    public:

        void try_open(const char* device)
        {
            assert(device);

            std::error_code error;
            Super::open(device, error);

            throw_if_error(error);
        }

        void try_request_resolution(uint32_t width, uint32_t height)
        {
            assert(width > 0);
            assert(height > 0);

            std::error_code error;
            Super::request_resolution(width, height, error);

            throw_if_error(error);

        }

        void try_request_i_frame_period(uint32_t i_frame_period)
        {
            std::error_code error;
            Super::request_i_frame_period(i_frame_period, error);

            throw_if_error(error);
        }

        void try_request_bitrates(uint32_t average_bitrate,
                                  uint32_t peak_bitrate)
        {
            std::error_code error;
            Super::request_bitrates(average_bitrate, peak_bitrate, error);

            throw_if_error(error);
        }

        void try_start_streaming()
        {
            std::error_code error;
            Super::start_streaming(error);

            throw_if_error(error);
        }

        void try_stop_streaming()
        {
            std::error_code error;
            Super::stop_streaming(error);

            throw_if_error(error);
        }

        capture_data try_capture()
        {
            std::error_code error;
            auto data = Super::capture(error);

            throw_if_error(error);
            return data;
        }

    };
}
