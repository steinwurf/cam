// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <cassert>
#include <ostream>

namespace cam
{
    /// Captured data from the camera
    struct capture_data
    {
        /// Default constructor
        capture_data() : m_data(nullptr), m_size(0), m_timestamp(0)
        { }

        /// Construct a new capture data
        capture_data(const uint8_t* data, uint32_t size, uint64_t timestamp)
            : m_data(data), m_size(size), m_timestamp(timestamp)
        {
            assert(m_data);
            assert(m_size > 0);
        }

        /// @return True if the capture data is valid otherwise false
        operator bool() const
        {
            if (m_data == nullptr)
                return false;

            if (m_size == 0)
                return false;

            return true;
        }

        /// Pointer to the captured memory buffer
        const uint8_t* m_data;

        /// Size of the captured memory buffer in bytes
        uint32_t m_size;

        /// Timestamp indicating when the data was captured in
        /// microseconds, starting from zero at the initial capture
        uint64_t m_timestamp;
    };

    /// Output operator for the capture data struct
    inline std::ostream& operator<<(std::ostream& os, const capture_data& data)
    {
        if (!data)
        {
            os << "cam::capture_data: invalid struct";
            return os;
        }

        os << "cam::capture_data: m_data = " << (void*) data.m_data << " "
           << "m_size = " << data.m_size << " m_timestamp = "
           << data.m_timestamp;

        return os;
    }
}
