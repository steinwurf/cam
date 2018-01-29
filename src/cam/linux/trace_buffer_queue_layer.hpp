// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

namespace cam
{
namespace linux
{
    template<class Super>
    class trace_buffer_queue_layer : public Super
    {
    public:

        /// Called to initialize memory
        void start_streaming(std::error_code& error)
        {
            assert(!error);

            if (Super::is_trace_enabled())
            {
                Super::write_trace("buffer_queue_layer", "start_streaming");
            }

            Super::start_streaming(error);
        }

        void stop_streaming(std::error_code& error)
        {
            assert(!error);

            if (Super::is_trace_enabled())
            {
                Super::write_trace("buffer_queue_layer", "stop_streaming");
            }

            Super::stop_streaming(error);
        }
        void enqueue_buffer(uint32_t index, std::error_code& error)
        {
            assert(!error);

            if (Super::is_trace_enabled())
            {
                std::stringstream ss;
                ss << "enqueue_buffer: index=" << index;
                Super::write_trace(
                    "buffer_queue_layer", ss.str());
            }

            Super::enqueue_buffer(index, error);
        }

        v4l2_buffer dequeue_buffer(std::error_code& error)
        {
            assert(!error);
            auto buffer = Super::dequeue_buffer(error);

            if (Super::is_trace_enabled())
            {
                assert(!error);
                std::stringstream ss;
                ss << "dequeu_buffer: index=" << buffer.index;
                Super::write_trace(
                    "buffer_queue_layer", ss.str());
            }
            return buffer;
        }
    };
}
}
