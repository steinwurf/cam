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

namespace c4m
{
    /// Small struct representing an Annex B NALU
    struct annex_b_nalu
    {
        /// Constructor ensuring the NALU struct is zero initialized
        annex_b_nalu() :
            m_data(nullptr),
            m_size(0),
            m_startcode_size(0)
        { }

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
        const uint8_t *m_data;

        /// Size of NALU in bytes
        uint32_t m_size;

        /// Size of the Annex B start code
        uint32_t m_startcode_size;
    };

    /// Output operator for the annex_b_nalu struct
    std::ostream& operator<<(std::ostream& os, const annex_b_nalu& nalu)
    {
        if (!nalu)
        {
            os << "c4m::annex_b_nalu: invalid struct";
            return os;
        }

        uint8_t nalu_type = nalu_type_from_header(
            nalu.m_data[nalu.m_startcode_size]);

        os << "c4m::annex_b_nalu: m_data = " << (void*) nalu.m_data << " "
           << "m_size = " << nalu.m_size << " m_startcode_size = "
           << nalu.m_startcode_size << " (type = "
           << nalu_type_to_string(nalu_type) << ")";

        return os;
    }
}
