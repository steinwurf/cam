// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "error.hpp"

#include <cassert>
#include <string>
#include <system_error>

namespace cam
{
    /// The error category
    class error_category : public std::error_category
    {
    public:

        /// @return The name of the category
        const char* name() const noexcept override
        {
            return "cam";
        }

        /// @return Human readable message corresponding to an error code
        std::string message(int ev) const override
        {
            switch(static_cast<error>(ev))
            {
                #define CAM_ERROR_TAG(id,msg) case error::id: return msg;
                #include "error_tags.hpp"
                #undef CAM_ERROR_TAG
            }

            assert(0 && "Invalid error code received!");
            return "";
        }

        /// @return A reference to the error category
        static const std::error_category& instance()
        {
            static error_category instance;
            return instance;
        }
    };
}
