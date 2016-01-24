// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <c4m/annex_b_nalu_parser.hpp>

#include <type_traits>
#include <gtest/gtest.h>

/// In the following we test whether our parse can correctly identify the
/// different NALUs produced by an Annex B encoder. We check we can find
/// the right address of the NALU plus the size of the start code.
TEST(test_c4m_annexb_nalu_parser, single_nalu)
{
    static const uint8_t nalu_data[] =
        { 0x00, 0x00, 0x00, 0x01, 0x12, 0xab };

    c4m::annex_b_nalu_parser parser(nalu_data, sizeof(nalu_data));

    // The first NALU at that start of the data
    EXPECT_EQ(parser.nalu(), nalu_data);
    EXPECT_EQ(parser.startcode_size(), 4U);

    parser.advance();
    EXPECT_TRUE(parser.at_end());
}

/// Test that we can find multiple NALUs
TEST(test_c4m_annexb_nalu_parser, multiple_nalu)
{
    static const uint8_t nalu_data[] =
        { 0x00, 0x00, 0x00, 0x01, 0x12, 0xab,
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab,
          0x00, 0x00, 0x01, 0x12, 0xab,        // <- 3 byte start code
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab};

    c4m::annex_b_nalu_parser parser(nalu_data, sizeof(nalu_data));

    // The first NALU at that start of the data
    EXPECT_EQ(parser.nalu(), nalu_data);
    EXPECT_EQ(parser.startcode_size(), 4U);

    parser.advance();
    EXPECT_FALSE(parser.at_end());

    EXPECT_EQ(parser.nalu(), nalu_data + 6);
    EXPECT_EQ(parser.startcode_size(), 4U);

    parser.advance();
    EXPECT_FALSE(parser.at_end());

    EXPECT_EQ(parser.nalu(), nalu_data + 12);
    EXPECT_EQ(parser.startcode_size(), 3U);

    parser.advance();
    EXPECT_FALSE(parser.at_end());

    EXPECT_EQ(parser.nalu(), nalu_data + 17);
    EXPECT_EQ(parser.startcode_size(), 4U);

    parser.advance();
    EXPECT_TRUE(parser.at_end());
}

/// Test that we can alternate NALU startcode size
TEST(test_c4m_annexb_nalu_parser, alternate_startcode_size)
{
    static const uint8_t nalu_data[] =
        { 0x00, 0x00, 0x01, 0x12, 0xab,
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab,
          0x00, 0x00, 0x01, 0x12, 0xab,
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab};

    c4m::annex_b_nalu_parser parser(nalu_data, sizeof(nalu_data));


    EXPECT_EQ(parser.startcode_size(), 3U);

    parser.advance();
    EXPECT_FALSE(parser.at_end());

    EXPECT_EQ(parser.startcode_size(), 4U);

    parser.advance();
    EXPECT_FALSE(parser.at_end());

    EXPECT_EQ(parser.startcode_size(), 3U);

    parser.advance();
    EXPECT_FALSE(parser.at_end());

    EXPECT_EQ(parser.startcode_size(), 4U);

    parser.advance();
    EXPECT_TRUE(parser.at_end());
}
