// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "scoped_file_descriptor.hpp"

#include "../error.hpp"

#include <fcntl.h>

namespace cam
{
namespace linux
{
    /// This layer will open the file descriptor representing the webcam device
    template<class Super>
    class open_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);
            assert(Super::is_status_closed());

            m_file_descriptor = scoped_file_descriptor(::open(device, O_RDWR));

            if (!m_file_descriptor)
            {
                error.assign(errno, std::generic_category());
            }
        }

    protected:

        /// @return The contained file descriptor
        const scoped_file_descriptor& file_descriptor() const
        {
            return m_file_descriptor;
        }

    private:

        // The file descriptor associated with the device
        scoped_file_descriptor m_file_descriptor;

    };
}
}
