// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <system_error>
#include <sstream>

namespace cam
{
namespace linux
{

    template<class Super>
    class trace_xu_query_layer : public Super
    {
    public:

        void xu_query(uint8_t unit, uint8_t selector, uint8_t query, uint8_t* data,
                   std::error_code& error)
        {
            Super::xu_query(unit, selector, query, data, error);

            if (Super::is_trace_enabled())
            {

                std::stringstream ss;
                ss << "unit=" << (uint32_t) unit << " ";
                ss << "selector=" << (uint32_t) selector << " ";
                ss << "query=" << (uint32_t) query << " ";
                ss << "data=" << (void*) data << " ";
                ss << "error=" << error << " ";

                if (error)
                    ss << "error_message=" << error.message();

                Super::write_trace(
                    "xu_query_layer", ss.str());
            }
        }
    };
}
}
