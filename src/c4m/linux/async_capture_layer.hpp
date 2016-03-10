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
    /// This layer implements the capture function that is used by the user
    /// to read frames from the asynchronously camera.
    template<class Super>
    class async_capture_layer : public Super
    {
    public:

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
}
}
