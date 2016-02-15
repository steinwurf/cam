// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <vector>

#include <boost/asio.hpp>

#include "v4l2_timestamp_to_micro_seconds.hpp"

namespace c4m
{
namespace linux
{
    /// @todo move to own header
    template<class Super>
    class async_capture_layer : public Super
    {
    public:

        /// Implement the open call such that we can access the file descriptor
        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if (error)
                return;

            // assert(m_io != nullptr);

            // const auto& fd = Super::file_descriptor();
            // assert(fd);

            // m_io_descriptor =
            //     std::unique_ptr<boost::asio::posix::stream_descriptor>(
            //         new boost::asio::posix::stream_descriptor(
            //             *m_io, fd.native_handle()));
        }

        /// @tparam ReadHandler type usable in expression such as:
        ///
        ///             ReadHandler rh;
        ///             rh(boost::system::error_code());
        ///
        template<class ReadHandler>
        void async_capture(ReadHandler read_handler)
        {
            assert(m_io != nullptr);
            assert(Super::is_status_streaming());

            // Quick fix - had this in open but move here due to find camera
            if (!m_io_descriptor)
            {
                assert(m_io != nullptr);

                const auto& fd = Super::file_descriptor();
                assert(fd);

                m_io_descriptor =
                    std::unique_ptr<boost::asio::posix::stream_descriptor>(
                        new boost::asio::posix::stream_descriptor(
                            *m_io, fd.native_handle()));
            }



            assert(m_io_descriptor);

            m_io_descriptor->async_read_some(
                boost::asio::null_buffers(), read_handler);
        }

        /// @param io The io_service to use for the async operations
        void set_io_service(boost::asio::io_service* io)
        {
            assert(io != nullptr);
            assert(m_io == nullptr);
            m_io = io;
        }

    private:

        boost::asio::io_service* m_io = nullptr;
        std::unique_ptr<boost::asio::posix::stream_descriptor> m_io_descriptor;

    };


    /// This layer implements the capture function that is used by the user
    /// to read frames from the camera.
    template<class Super>
    class capture_layer : public Super
    {
    public:

        /// Implement that start_streaming function such that we can
        /// initialize this layer.
        void start_streaming(std::error_code& error)
        {
            assert(!error);
            assert(Super::is_status_open());

            Super::start_streaming(error);

            if (error)
            {
                return;
            }

            // We don't have anything to enqueue on the initial capture
            m_enqueue_on_capture = false;
            m_enqueue_index = 0;
        }

        /// Reads data from the camera and returns it to the user. This is
        /// a blocking call.
        capture_data capture(std::error_code& error)
        {
            assert(!error);
            assert(Super::is_status_streaming());

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

        /// Keeps track of whether we have a buffer to enqueue when calling
        /// the capture(...) function.
        bool m_enqueue_on_capture = false;

        /// The index of the buffer we should enqueue when capture(...) is
        /// called
        uint32_t m_enqueue_index = 0;
    };
}
}
