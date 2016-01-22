// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

namespace c4m
{
    /// Enumeration of different error codes, we use a bit of macro
    /// uglyness to makes this easy. PHK says this is ok so if you have a
    /// problem with it - take it up with him :)
    ///
    /// http://phk.freebsd.dk/time/20141116.html
    enum class error
    {
        #define C4M_ERROR_TAG(id,msg) id,
        #include "error_tags.hpp"
        #undef C4M_ERROR_TAG
    };
}
