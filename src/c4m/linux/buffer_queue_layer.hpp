// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>

#include "../capture_data.hpp"

namespace c4m
{
namespace linux
{
    /// Takes care of enqueue'ing and dequeue'ing buffers.
    template<class Super>
    class buffer_queue_layer : public Super
    {
    public:

        /// Called to initialize memory
        void start_streaming(std::error_code& error)
        {
            assert(!error);
            assert(m_is_enqueued.size() == 0);

            Super::start_streaming(error);

            if (error)
            {
                return;
            }

            assert(Super::buffer_count() > 0);

            // Reset the enqueued tracker
            m_is_enqueued.resize(Super::buffer_count(), false);

            enqueue_all_buffers(error);
        }

        void stop_streaming(std::error_code& error)
        {
            assert(!error);

            Super::stop_streaming(error);

            // Reset the buffers struct
            m_is_enqueued.resize(0);
        }

        void enqueue_all_buffers(std::error_code& error)
        {
            assert(!error);

            for(uint32_t i = 0; i < m_is_enqueued.size(); ++i)
            {
                if (is_buffer_enqueued(i))
                {
                    // Buffer already enqueued
                    continue;
                }

                enqueue_buffer(i, error);

                if (error)
                {
                    return;
                }

                assert(m_is_enqueued[i] == true);
            }
        }

        /// Enqueue a specific buffer
        ///
        /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-qbuf.html
        ///
        /// @param index The index of the buffer we wanr to enqueue
        /// @param error Error code will be set if an error occurs.
        ///
        void enqueue_buffer(uint32_t index, std::error_code& error)
        {
            assert(!is_buffer_enqueued(index));
            assert(!error);

            v4l2_buffer buffer;
            memset(&buffer, 0, sizeof(buffer));

            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_MMAP;
            buffer.index = index;

            Super::retry_ioctl(VIDIOC_QBUF, &buffer, error);

            // Just to check, that the driver didn't do something crazy
            assert(buffer.index == index);

            if (error)
            {
                return;
            }

            assert(index < m_is_enqueued.size());
            m_is_enqueued[index] = true;
        }

        /// Dequeue a buffer with video data
        ///
        /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-qbuf.html
        ///
        /// @param error Error code will be set if an error occurs.
        /// @return The v4l2_buffer struct representing the captured data
        ///
        v4l2_buffer dequeue_buffer(std::error_code& error)
        {
            assert(!error);
            assert(have_enqueued_buffers());

            v4l2_buffer buffer;
            memset(&buffer, 0, sizeof(buffer));

            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_MMAP;

            Super::retry_ioctl(VIDIOC_DQBUF, &buffer, error);

            if (error)
            {
                return buffer;
            }

            assert(is_buffer_enqueued(buffer.index));
            assert(buffer.index < m_is_enqueued.size());

            m_is_enqueued[buffer.index] = false;

            return buffer;
        }

        /// @param index The index of the buffer we want to check
        /// @return True if the buffer is already enqueued
        bool is_buffer_enqueued(uint32_t index) const
        {
            assert(index < m_is_enqueued.size());
            return m_is_enqueued[index];
        }

        bool have_enqueued_buffers() const
        {
            for(auto is_enqueued : m_is_enqueued)
            {
                if (is_enqueued)
                {
                    return true;
                }
            }
            return false;
        }


    private:

        /// Tracks which buffers are currently enqueued
        std::vector<bool> m_is_enqueued;

    };
}
}
