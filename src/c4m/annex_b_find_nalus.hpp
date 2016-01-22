// Copyright Steinwurf ApS 2016.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

#include "annex_b_nalu.hpp"
#include "annex_b_nalu_parser.hpp"

namespace c4m
{
    /// Finds the NALUs stored in the memory region.
    ///
    /// @param data Buffer contaning one or more Annex B encoded NALUs
    /// @param size The size of the buffer in bytes
    /// @return A vector containing the found NALUs
    inline std::vector<annex_b_nalu>
    annex_b_find_nalus(const uint8_t* data, uint32_t size)
    {
        std::vector<annex_b_nalu> nalus;

        c4m::annex_b_nalu_parser parser(data, data + size);

        const uint8_t* nalu_start = parser.nalu();
        const uint8_t* nalu_end = nullptr;

        while (!parser.at_end())
        {
            parser.advance();

            // The logic here basically is that the end of the NALU is the
            // beginning of the next NALU
            if (!parser.at_end())
            {
                nalu_end = parser.nalu();
            }
            else
            {
                nalu_end = data + size;
            }

            annex_b_nalu nalu;
            nalu.m_data = nalu_start;
            nalu.m_size = nalu_end - nalu_start;

            nalus.push_back(nalu);

            nalu_start = nalu_end;
        }

        return nalus;
    }

}
