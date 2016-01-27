// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <cassert>
#include <fstream>

namespace c4m
{
namespace linux
{
    /// locates the first camera with h264 capablities, or returns an empty
    /// string if no camera was found.
    std::string find_camera()
    {
        // A bold assumption has been made here; if the user has a h264 capable
        // camera, it's one of the first 42 connected cameras.
        for (uint32_t i = 0U; i < 42U; ++i)
        {
            std::stringstream ss;
            ss << "/dev/video" << i;
            auto camera_file = ss.str();
            std::fstream c(camera_file);

            // check if camera exists
            if (!c.good())
                continue;

            // check if camera has h264 capablities
            c4m::linux::camera2<c4m::default_features> cc;

            std::error_code error;
            cc.open(camera_file.c_str(), error);
            if (!error)
                return camera_file;

        }
        return "";
    }
}
}
