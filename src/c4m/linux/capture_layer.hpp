// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <vector>

#include "v4l2_timestamp_to_micro_seconds.hpp"

namespace c4m
{
namespace linux
{
    template<class Super>
    class capture_layer : public Super
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

            // We don't have anything to enqueue on the initial capture
            m_enqueue_on_capture = false;
            m_enqueue_index = 0;
        }

        capture_data capture(std::error_code& error)
        {
            assert(!error);

            if (m_enqueue_on_capture)
            {
                Super::enqueue_buffer(m_enqueue_index, error);
                m_enqueue_on_capture = false;
            }

            // This call will block
            auto buffer_info = Super::dequeue_buffer(error);

            if (error)
            {
                return capture_data();
            }

            uint32_t index = buffer_info.index;
            uint32_t bytesused = buffer_info.bytesused;
            uint64_t timestamp = v4l2_timestamp_to_micro_seconds(buffer_info);

            const auto& buffer = Super::buffer(index);

            capture_data data;
            data.m_data = buffer.data();
            data.m_size = bytesused;
            data.m_timestamp = timestamp;

            // Make sure we re-enqueue the buffer on the next capture call
            m_enqueue_on_capture = true;
            m_enqueue_index = index;

            return data;
        }

    private:

        bool m_enqueue_on_capture = false;
        uint32_t m_enqueue_index = 0;
    };
}
}
