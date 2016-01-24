// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cassert>
#include <cstdint>

namespace c4m
{
    /// Simple Annex B NALU parser i.e. finds the NALUs in some memory buffer
    ///
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
    struct annex_b_nalu_parser
    {
        /// @todo lets change to take start and size
        annex_b_nalu_parser(const uint8_t* start, const uint8_t* end)
            : m_start(start),
              m_end(end),
              m_cursor(start),
              m_startcode_size(0)
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
        // In the following we search for the start codes and return the
        // offset to the NALU header following the start code.
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
            m_cursor += m_startcode_size;

            for(;;++m_cursor)
            {
                // Looking for the 3 byte start code, so there must be at
                // least 4 bytes remaining
                if (m_end - m_cursor < 4)
                    break;

                if (m_cursor[0] != 0x00)
                    continue;

                if (m_cursor[1] != 0x00)
                    continue;

                if (m_cursor[2] == 0x01)
                {
                    m_startcode_size = 3;
                    return;
                }

                if (m_cursor[2] != 0x00)
                    continue;

                // Looking for the 4 byte start code, so there must be at
                // least 5 bytes remaining
                if (m_end - m_cursor < 5)
                    break;

                if(m_cursor[3] == 0x01)
                {
                    m_startcode_size = 4;
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

        /// @return The size of the start code of the current NALU
        uint32_t startcode_size() const
        {
            return m_startcode_size;
        }

    private:

        const uint8_t* m_start;
        const uint8_t* m_end;
        const uint8_t* m_cursor;

        uint32_t m_startcode_size;

    };
}
