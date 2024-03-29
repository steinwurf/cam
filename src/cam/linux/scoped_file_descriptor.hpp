// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstddef>
#include <cassert>

#include <unistd.h>

namespace cam
{
namespace linux
{
    struct close_policy
    {
        void close(int file_descriptor)
        {
            assert(file_descriptor >= 0);
            auto ok = ::close(file_descriptor);
            assert(ok == 0);
        }

    };

    /// RAII (Resource Allocation Is Initialization) wrapper for a file
    /// descriptor. Ensures that the file descriptor gets closed.
    //template<class ClosePolicy = close_policy>
    class scoped_file_descriptor
    {
    public:

        /// Default constructor for a non-owning scoped_file_descriptor
        /// i.e. it will not do anything when going out of scope.
        scoped_file_descriptor() : m_file_descriptor(-1)
        { }

        /// Take ownership of a "naked" file descriptor
        ///
        /// @param file_descriptor The file descriptor we will own.
        scoped_file_descriptor(int file_descriptor) :
            m_file_descriptor(file_descriptor)
        {
            assert(m_file_descriptor >= 0);
        }

        /// Take ownership of a file descriptor wrapped in a temporary
        /// scoped_file_descriptor
        ///
        /// @param other Temporary scoped_file_descriptor where we will
        ///              take ownership of the contained file descriptor
        scoped_file_descriptor(scoped_file_descriptor&& other)
        {
            m_file_descriptor = other.m_file_descriptor;
            other.m_file_descriptor = -1;
        }

        /// Take ownership of a file descriptor wrapped in a temporary
        /// scoped_file_descriptor
        ///
        /// @param other Temporary scoped_file_descriptor where we will
        ///              take ownership of the contained file descriptor
        scoped_file_descriptor& operator=(scoped_file_descriptor&& other)
        {
            m_file_descriptor = other.m_file_descriptor;
            other.m_file_descriptor = -1;
            return *this;
        }

        /// Destructor which will ensure that an owned file descriptor
        /// will be closed.
        ~scoped_file_descriptor()
        {
            if (m_file_descriptor != -1)
            {
                // We ignore any errors that can occur here - no clue if we
                // can do something better?
                ::close(m_file_descriptor);
            }
        }

        /// @return The contained "naked" file descriptor.
        int native_handle() const
        {
            assert(m_file_descriptor != -1);
            return m_file_descriptor;
        }

        /// @return True if a valid file descriptor is contained otherwise
        ///         false.
        explicit operator bool() const
        {
            return m_file_descriptor != -1;
        }

    private:

        /// Delete the copy and copy assignment constructors - we cannot
        /// have two scoped_file_descriptors owning the same file
        /// descriptor.
        scoped_file_descriptor(const scoped_file_descriptor&) = delete;

        scoped_file_descriptor& operator=(
            const scoped_file_descriptor&) = delete;

    private:

        /// The owned file descriptor
        int m_file_descriptor;
    };
}
}
