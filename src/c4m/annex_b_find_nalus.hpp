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

        // Get information for the first NALU
        const uint8_t* nalu_start = parser.nalu();
        const uint8_t* nalu_end = nullptr;

        while (!parser.at_end())
        {
            // Fetch the start code size before we search for the next NALU
            // in order to find the end.
            uint32_t startcode_size = parser.startcode_size();

            // Find the next NALU such that we know where the current ends
            parser.advance();

            // The logic here basically is that the end of the NALU is the
            // beginning of the next NALU
            if (!parser.at_end())
            {
                nalu_end = parser.nalu();
            }
            else
            {
                // No more NALUs so we set the end as the end of the buffer
                nalu_end = data + size;
            }

            annex_b_nalu nalu;
            nalu.m_data = nalu_start;
            nalu.m_size = nalu_end - nalu_start;
            nalu.m_startcode_size = startcode_size;

            nalus.push_back(nalu);

            nalu_start = nalu_end;
        }

        return nalus;
    }

}
