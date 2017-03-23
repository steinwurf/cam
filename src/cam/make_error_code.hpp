// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "error.hpp"
#include "error_category.hpp"

namespace cam
{
    /// @return an error_code with beem errors
    inline std::error_code make_error_code(error e)
    {
        return std::error_code(static_cast<int>(e), error_category::instance());
    }
}
