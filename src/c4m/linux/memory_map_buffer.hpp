// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <sys/mman.h>

namespace c4m
{
namespace linux
{
    /// RAII helper for storing memory mapped pointers. Upon destruction
    /// the memory will be un-mapped.
    class memory_map_buffer
    {
    public:

        memory_map_buffer(uint8_t* data, uint32_t size)
            : m_data(data),
              m_size(size)
        {
            assert(m_data);
            assert(m_data != MAP_FAILED);
            assert(m_size > 0);
        }

        memory_map_buffer(memory_map_buffer&& u)
        {
            m_data = u.m_data;
            m_size = u.m_size;

            u.m_data = (uint8_t*) MAP_FAILED;
            u.m_size = 0;
        }

        ~memory_map_buffer()
        {
            if (m_data != MAP_FAILED)
            {
                assert(m_size > 0);
                // We cannot do much about the errors here
                ::munmap(m_data, m_size);
            }
        }

        const uint8_t* data() const
        {
            assert(m_data);
            assert(m_data != MAP_FAILED);
            return m_data;
        }

        uint32_t size() const
        {
            assert(m_size > 0);
            return m_size;
        }

        explicit operator bool() const
        {
            return m_data != MAP_FAILED;
        }

    private:

        memory_map_buffer(const memory_map_buffer&) = delete;
        memory_map_buffer& operator=(const memory_map_buffer&) = delete;

    private:

        uint8_t* m_data;
        uint32_t m_size;
    };
}
}
