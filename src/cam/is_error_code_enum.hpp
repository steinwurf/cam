// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>

#include "error.hpp"

namespace std
{
    /// Reference: http://bit.ly/1RCq7ts
    template <>
    struct is_error_code_enum<cam::error> : public true_type {};
}
