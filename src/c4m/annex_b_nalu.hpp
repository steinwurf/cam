// Copyright Steinwurf ApS 2016.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <cassert>
#include <cstdint>

namespace c4m
{
    /// Small struct representing an Annex B NALU
    struct annex_b_nalu
    {
        /// Constructor ensuring the NALU struct is zero initialized
        annex_b_nalu() :
            m_data(nullptr),
            m_size(0)
        { }

        /// Pointer to the NALU data
        const uint8_t *m_data;

        /// Size of NALU in bytes
        uint32_t m_size;
    };
}
