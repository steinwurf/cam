// Copyright Steinwurf ApS 2016.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <cassert>
#include <cstdint>
#include <ostream>

#include "nalu_type_from_header.hpp"
#include "nalu_type_to_string.hpp"
#include "nalu_type.hpp"

namespace c4m
{
    /// Small struct representing an Annex B NALU
    struct annex_b_nalu
    {
        /// @return True if the NALU is valid otherwise false
        operator bool() const
        {
            if (m_data == nullptr)
                return false;

            if (m_size == 0)
                return false;

            // A valid start code is either 3 or 4 bytes
            if (m_startcode_size != 3 && m_startcode_size != 4)
                return false;

            return true;
        }

        /// Pointer to the NALU data
        const uint8_t *m_data = nullptr;

        /// Size of NALU in bytes
        uint32_t m_size = 0;

        /// Size of the Annex B start code
        uint32_t m_startcode_size = 0;

        /// The type of the NALU
        nalu_type m_type;
    };

    /// Output operator for the annex_b_nalu struct
    std::ostream& operator<<(std::ostream& os, const annex_b_nalu& nalu)
    {
        if (!nalu)
        {
            os << "c4m::annex_b_nalu: invalid struct";
            return os;
        }

        os << "c4m::annex_b_nalu: m_data = " << (void*) nalu.m_data << " "
           << "m_size = " << nalu.m_size << " m_startcode_size = "
           << nalu.m_startcode_size << " (type = "
           << nalu_type_to_string(nalu.m_type) << ")";

        return os;
    }
}
