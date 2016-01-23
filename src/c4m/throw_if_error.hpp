// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "capture_data.hpp"

namespace c4m
{
    /// Small helper function which throws an exception if the error code
    /// passed indicates an error.
    inline void throw_if_error(const std::error_code& error)
    {
        if (error)
        {
            throw std::system_error(error);
        }
    }


    template<class Super>
    class throw_if_error_layer : public Super
    {
    public:

        void open(const char* device)
        {
            assert(device);

            std::error_code error;
            Super::open(device, error);

            throw_if_error(error);
        }

        void request_resolution(uint32_t width, uint32_t height)
        {
            assert(width > 0);
            assert(height > 0);

            std::error_code error;
            Super::request_resolution(width, height, error);

            throw_if_error(error);

        }

        void start_streaming()
        {
            std::error_code error;
            Super::start_streaming(error);

            throw_if_error(error);
        }

        void stop_streaming()
        {
            std::error_code error;
            Super::stop_streaming(error);

            throw_if_error(error);
        }

        capture_data capture()
        {
            std::error_code error;
            auto data = Super::capture(error);

            throw_if_error(error);
            return data;
        }

    };
}
