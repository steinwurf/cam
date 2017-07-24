// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cassert>
#include <system_error>

namespace cam
{
    /// Updates the status_layer depending on the error values returned by
    /// the underlying operations.
    template<class Super>
    class update_status_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);
            assert(Super::is_status_closed());

            Super::open(device, error);

            if(!error)
            {
                Super::set_status_open();
            }
        }

        void start_streaming(std::error_code& error)
        {
            assert(!error);
            assert(Super::is_status_open());

            Super::start_streaming(error);

            if (!error)
            {
                Super::set_status_streaming();
            }
        }

        void stop_streaming(std::error_code& error)
        {
            assert(!error);
            assert(Super::is_status_streaming());

            Super::stop_streaming(error);

            if (!error)
            {
                // Set the status to open - not closed as we didn't close the
                // streaming we just stopped it.
                Super::set_status_open();
            }
        }

    };
}
