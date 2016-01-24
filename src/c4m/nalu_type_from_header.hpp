// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

namespace c4m
{
    /// From RFC 6184 (https://tools.ietf.org/html/rfc6184#section-1.3)
    ///
    /// For convenience, the format of the NAL unit header is
    /// reprinted below:
    ///
    ///      +---------------+
    ///      |0|1|2|3|4|5|6|7|
    ///      +-+-+-+-+-+-+-+-+
    ///      |F|NRI|  Type   |
    ///      +---------------+
    ///
    /// To extract type we use bitmask: 0001 1111 = 0x1F
    ///
    uint8_t nalu_type_from_header(uint8_t nalu_header)
    {
        uint8_t type = nalu_header & 0x1F;
        return type;
    }
}
