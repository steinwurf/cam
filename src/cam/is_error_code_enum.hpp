// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <system_error>

#include "error.hpp"

namespace std
{
    /// Reference: http://bit.ly/1RCq7ts
    template <>
    struct is_error_code_enum<cam::error> : public true_type {};
}
