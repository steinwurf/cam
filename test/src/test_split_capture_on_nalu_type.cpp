// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#include <c4m/split_capture_on_nalu_type.hpp>

#include <gtest/gtest.h>

TEST(test_split_capture_on_nalu_type, single_nalu)
{
    static const uint8_t nalu_data[] =
        { 0x00, 0x00, 0x00, 0x01, 0x12, 0xab };

}

TEST(test_split_capture_on_nalu_type, multiple_nalu)
{
    static const uint8_t nalu_data[] =
        { 0x00, 0x00, 0x00, 0x01, 0x12, 0xab,
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab, 0xff,
          0x00, 0x00, 0x01, 0x12, 0xab,        // <- 3 byte start code
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab};
}


TEST(test_split_capture_on_nalu_type, alternate_startcode_size)
{
    static const uint8_t nalu_data[] =
        { 0x00, 0x00, 0x01, 0x12, 0xab,
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab, 0xff,
          0x00, 0x00, 0x01, 0x12, 0xab,        // <- 3 byte start code
          0x00, 0x00, 0x00, 0x01, 0x12, 0xab};


}
