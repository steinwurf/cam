// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>

namespace c4m
{
namespace linux
{
    template<class Super>
    class zero_timestamp_at_initial_capture : public Super
    {
    public:

        void start_streaming(std::error_code& error)
        {
            assert(!error);

            Super::start_streaming(error);

            if (error)
            {
                return;
            }

            // We are starting a capture
            m_initial_capture = true;
            m_initial_timestamp = 0;
        }

        capture_data capture(std::error_code& error)
        {
            assert(!error);

            auto data = Super::capture(error);

            if (error)
            {
                return capture_data();
            }

            if (m_initial_capture)
            {
                m_initial_timestamp = data.m_timestamp;
                m_initial_capture = false;
            }

            data.m_timestamp = data.m_timestamp - m_initial_timestamp;

            return data;

        }

    private:

        bool m_initial_capture = true;
        uint64_t m_initial_timestamp = 0;
    };
}
}
