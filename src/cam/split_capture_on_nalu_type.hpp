// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "capture_data.hpp"

#include <nalu/to_annex_b_nalus.hpp>

#include <vector>

namespace cam
{
    /// Takes as input a capture_data struct and returns a vector of
    /// capture_data structs, ensuring that the SPS and PPS NALUs are
    /// returned in seperate capture data.
    ///
    /// @param data The capture data that should be split if a SPS or PPS
    ///        is found
    /// @return The vector of capture_data after the split
    inline std::vector<capture_data>
    split_capture_on_nalu_type(const capture_data& data)
    {
        std::vector<capture_data> split_capture;

        auto nalus = nalu::to_annex_b_nalus(data.m_data, data.m_size);

        capture_data aggregate;

        for (const auto& nalu : nalus)
        {
            if (nalu.m_type == nalu::type::sequence_parameter_set ||
                nalu.m_type == nalu::type::picture_parameter_set)
            {
                if (aggregate)
                {
                    // If we already have aggregated some data we push that
                    // back and reset the aggregate before pushing the SPS or
                    // PPS separately
                    split_capture.push_back(aggregate);
                    aggregate = capture_data();
                }

                // Push the SPS or PPS as separate capture data
                capture_data ps(nalu.m_data, nalu.m_size, data.m_timestamp);
                split_capture.push_back(ps);

                // Next NALU
                continue;
            }

            if (!aggregate)
            {
                // No aggregate data yet, so we start from this NALU
                aggregate = capture_data(nalu.m_data, nalu.m_size,
                                         data.m_timestamp);
            }
            else
            {
                // We already have an aggregate so add this NALU
                aggregate.m_size += nalu.m_size;
            }
        }

        if (aggregate)
        {
            split_capture.push_back(aggregate);
        }

        return split_capture;
    }
}
