// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <c4m/annex_b_find_nalus.hpp>

#include <gtest/gtest.h>

/// In the following we test whether our parse can correctly identify the
/// different NALUs produced by an Annex B encoder. We check we can find
/// the right address of the NALU plus the size of the start code.
TEST(test_annex_b_find_nalus, single_nalu)
{
    static const uint8_t nalu_data[] =
        { 0x00, 0x00, 0x00, 0x01, 0x12, 0xab };

    auto nalus = c4m::annex_b_find_nalus(nalu_data, sizeof(nalu_data));

    EXPECT_EQ(nalus.size(), 1U);

    // The first NALU at that start of the data
    auto nalu_0 = nalus[0];

    EXPECT_EQ(nalu_0.m_data, nalu_data);
    EXPECT_EQ(nalu_0.m_size, 6U);
    EXPECT_EQ(nalu_0.m_startcode_size, 4U);
}

TEST(test_annex_b_find_nalus, multiple_nalu)
{
    static const uint8_t nalu_data[] =
        { 0x00, 0x00, 0x00, 0x01, 0x12, 0xab,
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab, 0xff,
          0x00, 0x00, 0x01, 0x12, 0xab,        // <- 3 byte start code
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab};

    auto nalus = c4m::annex_b_find_nalus(nalu_data, sizeof(nalu_data));

    EXPECT_EQ(nalus.size(), 4U);

    // Lets check the different NALUs
    auto nalu_0 = nalus[0];

    EXPECT_EQ(nalu_0.m_data, nalu_data);
    EXPECT_EQ(nalu_0.m_size, 6U);
    EXPECT_EQ(nalu_0.m_startcode_size, 4U);

    auto nalu_1 = nalus[1];

    EXPECT_EQ(nalu_1.m_data, nalu_data + 6);
    EXPECT_EQ(nalu_1.m_size, 7U);
    EXPECT_EQ(nalu_1.m_startcode_size, 4U);

    auto nalu_2 = nalus[2];

    EXPECT_EQ(nalu_2.m_data, nalu_data + 13);
    EXPECT_EQ(nalu_2.m_size, 5U);
    EXPECT_EQ(nalu_2.m_startcode_size, 3U);

    auto nalu_3 = nalus[3];

    EXPECT_EQ(nalu_3.m_data, nalu_data + 18);
    EXPECT_EQ(nalu_3.m_size, 6U);
    EXPECT_EQ(nalu_3.m_startcode_size, 4U);
}


TEST(test_annex_b_find_nalus, alternate_startcode_size)
{
    static const uint8_t nalu_data[] =
        { 0x00, 0x00, 0x01, 0x12, 0xab,
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab, 0xff,
          0x00, 0x00, 0x01, 0x12, 0xab,        // <- 3 byte start code
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab};

    auto nalus = c4m::annex_b_find_nalus(nalu_data, sizeof(nalu_data));

    EXPECT_EQ(nalus.size(), 4U);

    // Lets check the different NALUs
    auto nalu_0 = nalus[0];
    EXPECT_EQ(nalu_0.m_startcode_size, 3U);

    auto nalu_1 = nalus[1];
    EXPECT_EQ(nalu_1.m_startcode_size, 4U);

    auto nalu_2 = nalus[2];
    EXPECT_EQ(nalu_2.m_startcode_size, 3U);

    auto nalu_3 = nalus[3];
    EXPECT_EQ(nalu_3.m_startcode_size, 4U);
}
