// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <c4m/split_capture_on_nalu_type.hpp>
#include <c4m/capture_data.hpp>

#include <vector>

#include <gtest/gtest.h>

TEST(test_split_capture_on_nalu_type, single_nalu)
{
    std::vector<uint8_t> nalu_data =
    {
        0x00, 0x00, 0x00, 0x01, 0x12, 0xab
    };
    uint32_t timestamp = 100;
    auto capture_data = c4m::capture_data(
        nalu_data.data(), nalu_data.size(), timestamp);
    auto nalus = c4m::split_capture_on_nalu_type(capture_data);
    EXPECT_EQ(1U, nalus.size());
}

TEST(test_split_capture_on_nalu_type, multiple_nalu)
{
    std::vector<uint8_t> nalu_data =
    {
        0x00, 0x00, 0x00, 0x01, 0x12, 0xab,
        0x00, 0x00, 0x00, 0x01, 0x12, 0xab, 0xff,
        0x00, 0x00, 0x00, 0x01, 0x12, 0xab,
        0x00, 0x00, 0x00, 0x01, 0x12, 0xab
    };
    uint32_t timestamp = 100;
    auto capture_data = c4m::capture_data(
       nalu_data.data(), nalu_data.size(), timestamp);
    auto nalus = c4m::split_capture_on_nalu_type(capture_data);
    ///@todo fix?
    // EXPECT_EQ(4U, nalus.size());
}


TEST(test_split_capture_on_nalu_type, alternate_startcode_size)
{
    std::vector<uint8_t> nalu_data =
    {
        0x00, 0x00, 0x01, 0x12, 0xab,
        0x00, 0x00, 0x00, 0x01, 0x12, 0xab, 0xff,
        0x00, 0x00, 0x01, 0x12, 0xab,        // <- 3 byte start code
        0x00, 0x00, 0x00, 0x01, 0x12, 0xab
    };
    uint32_t timestamp = 100;
    auto capture_data = c4m::capture_data(
        nalu_data.data(), nalu_data.size(), timestamp);
    auto nalus = c4m::split_capture_on_nalu_type(capture_data);
    ///@todo fix?
    // EXPECT_EQ(4U, nalus.size());
}
