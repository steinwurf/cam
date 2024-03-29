// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <cam/split_capture_on_nalu_type.hpp>
#include <cam/capture_data.hpp>

#include <vector>

#include <gtest/gtest.h>

TEST(test_split_capture_on_nalu_type, single_nalu)
{
    std::vector<uint8_t> nalu_data =
    {
        0x00, 0x00, 0x00, 0x01, 0x12, 0xab
    };
    uint32_t timestamp = 100;
    auto capture_data = cam::capture_data(
        nalu_data.data(), nalu_data.size(), timestamp);
    auto nalus = cam::split_capture_on_nalu_type(capture_data);
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
    auto capture_data = cam::capture_data(
       nalu_data.data(), nalu_data.size(), timestamp);
    auto nalus = cam::split_capture_on_nalu_type(capture_data);
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
    auto capture_data = cam::capture_data(
        nalu_data.data(), nalu_data.size(), timestamp);
    auto nalus = cam::split_capture_on_nalu_type(capture_data);
    ///@todo fix?
    // EXPECT_EQ(4U, nalus.size());
}
