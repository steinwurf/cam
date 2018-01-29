// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cassert>
#include <cstdint>
#include <linux/uvcvideo.h>
#include <linux/usb/video.h>



namespace cam
{
namespace linux
{

template<class Super>
class xu_query_layer : public Super
{
public:

    void xu_query(uint8_t unit, uint8_t selector, uint8_t query,
                  uint8_t* data, std::error_code& error)
    {
        assert(data);
        assert(!error);

        uvc_xu_control_query xu;
        memset(&xu, 0, sizeof(xu));

        uint16_t length = 0;

        xu.unit = unit;
        xu.selector = selector;
        xu.query = UVC_GET_LEN;
        xu.size = sizeof(length);
        xu.data = (uint8_t*) &length;

        Super::retry_ioctl(UVCIOC_CTRL_QUERY, &xu, error);

        if (error)
            return;

        if (query == UVC_GET_LEN)
        {
            *((uint16_t*)data) = length;
            return;
        }

        xu.query = query;
        xu.size = length;
        xu.data = data;

        Super::retry_ioctl(UVCIOC_CTRL_QUERY, &xu, error);
    }
};
}
}
