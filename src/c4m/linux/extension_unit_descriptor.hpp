// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>

namespace c4m
{
namespace linux
{
    /// The extension unit descriptor.
    ///
    /// Defined in:
    ///    UVC 1.5 Class specification.pdf
    ///
    /// Section 3.7.2.7 (p. 58)
    ///
    struct extension_unit_descriptor
    {
        // Size of this descriptor in bytes
        int8_t m_length;

        /// Constant of CS_INTERFACE descriptor type
        int8_t m_descriptor_type;

        /// Constant of VC_EXTENSION_UNIT descriptor subtype
        int8_t m_descriptor_sub_type;

        // Unique identifier for unit, this value must be passed with each
        // request that is directed to the Extension Unit.
        int8_t m_unit_id;

        // Vendor specific code that further identifies the extension unit.
        uint8_t m_guid_extension_code[16];
    } __attribute__ ((__packed__));
}
}
