// Copyright Steinwurf ApS 2011.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <cassert>
#include <cstdint>
#include <ostream>
#include <iomanip>

#include <sak/storage.hpp>

/// Note on the Emulation Prevention bytes:
///
///     In short the following values are illegal (0x000000, 0x000001,
///     0x000002) in the data of a H264 NALU. Therefore these vales are
///     escaped using an 'Emulation Prevention' byte 0x03. See more:
///     http://stackoverflow.com/a/24890903
///
///     This brings us to the question should we remove these 'Emulation
///     Prevention' bytes when sending an raw stram. According to (luckely)
///     we don't have to: http://bit.ly/1ZMwIWW.
///
///     This is backed by http://bit.ly/1PFeaNH who states that 'Emulation
///     Prevention' bytes are part of the standard H264 and not something
///     defined by Annex B. Therefore a raw H264 stream should contain
///     'Emulation Prevention' bytes.
///

namespace temp
{
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



    // template<class
    // struct n

//     const char* nalu_type_to_string(uint8_t nalu_type)
//     {
// //         static const char* types[] =
// //             {
// // #define NALU_TAG(id, type, vcl) type,
// // #include "nalu_tag.hpp"
// // #undef NALU_TAG
// //                 "terminate"
// //             };

//         return types[nalu_type];
//     }



    void next_nalu(const uint8_t* start, const uint8_t* end)
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

            uint8_t type = nalu_type(nalu_header);

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

    // void next_nalu(const uint8_t* start, const uint8_t* end)
    // {
    //     assert(start != nullptr);
    //     assert(end != nullptr);
    //     assert(start < end);

    //     const uint8_t* next = next_nalu_offset(start, end);

    //     assert(start < next);

    //     while(next != end)
    //     {
    //         uint32_t offset = next - start;

    //         uint8_t nalu_header = *next;

    //         uint8_t type = nalu_type(nalu_header);

    //         std::cout << "NALU type = " << (uint32_t) type << " offset = "
    //                   << offset << " header value (hex) = "
    //                   << std::hex << (uint32_t) nalu_header << std::dec
    //                   << " header value (dec) = " << (uint32_t) nalu_header
    //                   << std::endl;
    //                   // << " string = " << nalu_type_to_string(type) << std::endl;

    //         uint8_t forbidden = nalu_header & 0x80;
    //         assert(forbidden == 0 && "first bit of NALU header must be zero");

    //         new_next = next_nalu_offset(next, end);

    //         std::

    //         assert(start < next);
    //     }
    // }


////////////////////////////////////////////////////////////////////////////////////

    struct annexb_nalu_parser
    {

        annexb_nalu_parser(const uint8_t* start, const uint8_t* end)
            : m_start(start),
              m_end(end),
              m_cursor(start),
              m_start_code_size(0)
        {
            assert(m_start);
            assert(m_end);
            assert(m_cursor);
            assert(m_end > m_start);

            // Find the first NALU, we expect it to be located at start,
            // but we don't know
            advance();
        }

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
        void advance()
        {
            assert(m_start != nullptr);
            assert(m_end != nullptr);
            assert(m_end > m_start);

            // Jump over the start code; When initializing the parser
            // m_cursor equals m_start and the m_start_code is zero - so we
            // start from the beginning
            m_cursor += m_start_code_size;

            uint32_t size = m_end - m_cursor;

            assert(size > 3U && "A 3 byte start code must be followed "
                   "by at least one byte. So size must be larger than 3.");

            for(;;++m_cursor)
            {
                // Looking for the 3 byte start code, so there must be at
                // least 4 bytes remaining
                if (m_end - m_cursor > 3)
                    break;

                if (m_cursor[0] != 0x00)
                    continue;

                if (m_cursor[1] != 0x00)
                    continue;

                if (m_cursor[2] == 0x01)
                {
                    m_start_code_size = 3;
                    return;
                }

                if (m_cursor[2] != 0x00)
                    continue;

                // Looking for the 4 byte start code, so there must be at
                // least 5 bytes remaining
                if (m_end - m_cursor > 4)
                    break;

                if(m_cursor[3] == 0x01)
                {
                    m_start_code_size = 4;
                    return;
                }
            }

            // We did not find any new start codes and we ran out of data
            // to parse so we must be at the end.
            m_cursor = m_end;
        }

        /// @return Pointer to the beginning to the current NALU
        const uint8_t* nalu() const
        {
            return m_cursor;
        }

        /// @return true if there are no more NALUs to find, otherwise
        ///         false and we can continue searching
        bool at_end() const
        {
            return m_cursor == m_end;
        }

    private:

        const uint8_t* m_start;
        const uint8_t* m_end;
        const uint8_t* m_cursor;

        uint32_t m_start_code_size;

    };
}
