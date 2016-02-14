// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

namespace c4m
{
    /// The layer keeps track of the status of the camera. To manage this
    /// the update_status_layer will automatically move the camera though
    /// the different states based on the error_code values of the
    /// different operations.
    template<class Super>
    class status_layer : public Super
    {
    private:

        /// The different states the camera can be in.
        enum class status
        {
            closed,
            open,
            streaming
        };

    public:

        /// Moves the camera to the open state
        void set_status_open()
        {
            m_status = status::open;
        }

        /// Moves the camera to the closed state
        void set_status_closed()
        {
            m_status = status::closed;
        }

        /// Moves the camera to the streaming state
        void set_status_streaming()
        {
            m_status = status::streaming;
        }

        /// @return True if the camera currently is open
        bool is_status_open() const
        {
            return m_status == status::open;
        }

        /// @return True if the camera currently is closed
        bool is_status_closed() const
        {
            return m_status == status::closed;
        }

        /// @return True if the camera currently is streaming
        bool is_status_streaming() const
        {
            return m_status == status::streaming;
        }

    private:

        /// Keeps track of the status
        status m_status = status::closed;
    };
}
