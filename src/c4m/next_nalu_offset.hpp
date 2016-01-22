// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

namespace c4m
{
    /// In Annex B a NALU is prefixed by either a 3 or 4 byte start code:
    ///
    ///    4 byte variant: 0x00000001
    ///    3 byte variant: 0x000001
    ///
    /// The following:
    struct nalu
    {
        nalu() : m_nalu_start(nullptr),
                 m_nalu_end(nullptr),
                 m_data_start(nullptr)
        { }

        /// Pointer to the start of the NALU (including the start code).
        uint8_t* m_nalu_start;

        /// Pointer to the end of the NALU (points to the end-of-stream or
        /// the start code of the next NALU).
        uint8_t* m_nalu_end;

        /// Pointer to the data portion of the NALU i.e. skipping over the
        /// start code
        uint8_t* m_data_start;




        uint32_t nalu_size() const
        {
            assert(0);
            return m_nalu_end - m_nalu_start;
        }

        operator bool()()
        {
            if (m_nalu_start == nullptr)
                return false;

            if(m_nalu_end == nullptr)
                return false;

            if (m_data_start == nullptr)
                return false;

            // Check forbidden bit
            // assert(m_data_start != nullptr);
            // uint8_t type = nalu_type(*m_data_start);

            // uint8_t forbidden = nalu_header & 0x80;
            // assert(forbidden == 0 && "first bit of NALU header must be zero")

            return true;
        }

    };

    // In Annex B a NALU is prefixed by either a 3 or 4 byte start code:
    //
    //    4 byte variant: 0x00000001
    //    3 byte variant: 0x000001
    //
    // In the following we search for the start codes and return the offset to
    // the NALU header following the start code.
    //
    // @return The offset to the next NALU header, if no valid start code is
    //         found returns
    const uint8_t* next_nalu_offset(const uint8_t* start, const uint8_t* end)
    {
        assert(start != nullptr);
        assert(end != nullptr);
        assert(end > start);

        uint32_t size = end - start;

        assert(size > 3U && "A 3 byte start code must be followed "
               "by at least one byte");

        for(;;++start)
        {
            // Looking for the 3 byte start code, so there must be at least 4
            // bytes remaining
            if (start + 3 >= end)
                break;

            if (start[0] != 0x00)
                continue;

            if (start[1] != 0x00)
                continue;

            if (start[2] == 0x01)
                return start + 3;

            if (start[2] != 0x00)
                continue;

            // Looking for the 4 byte start code, so there must be at least 5
            // bytes remaining
            if (start + 4 >= end)
                break;

            if(start[3] == 0x01)
                return start + 4;
        }

        return end;
    }


    void find_nalus_nalu(const uint8_t* start, const uint8_t* end)
    {
        assert(start != nullptr);
        assert(end != nullptr);
        assert(start < end);

        const uint8_t* next = next_nalu_offset(start, end);

        assert(start < next);

        while(next != end)
        {
            uint32_t offset = next - start;

            uint8_t nalu_header = *next;



            std::cout << "NALU type = " << (uint32_t) type << " offset = "
                      << offset << " header value (hex) = "
                      << std::hex << (uint32_t) nalu_header << std::dec
                      << " header value (dec) = " << (uint32_t) nalu_header
                      << std::endl;
                      // << " string = " << nalu_type_to_string(type) << std::endl;

            uint8_t forbidden = nalu_header & 0x80;
            assert(forbidden == 0 && "first bit of NALU header must be zero");

            next = next_nalu_offset(next, end);

            assert(start < next);
        }
    }
}
