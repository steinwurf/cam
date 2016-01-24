// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "nalu_type.hpp"

namespace c4m
{
    /// @return Human readable message corresponding to an error code
    std::string nalu_type_to_string(uint8_t type)
    {
        switch(type)
        {
            #define C4M_NALU_TYPE_TAG(id,msg,vcl) \
                case id: return msg;
            #include "nalu_type_tags.hpp"
            #undef C4M_NALU_TYPE_TAG
        }

        assert(0 && "Invalid NALU type received!");
        return "";
    }
}
