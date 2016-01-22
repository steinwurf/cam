// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "retry_ioctl.hpp"

namespace c4m
{
namespace linux
{
    /// Enumerates the supported formats
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    std::error_code error;
    ///    auto formats = c4m::linux::enumerate_formats(fd, type, error);
    ///
    ///    if (error)
    ///    {
    ///        // Something when wrong
    ///    }
    ///
    /// Docs: https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-fmt.html
    ///
    inline std::vector<v4l2_fmtdesc>
    enumerate_formats(const scoped_file_descriptor& fd, v4l2_buf_type type,
                      std::error_code& error)
    {
        assert(fd);

        std::vector<v4l2_fmtdesc> formats;
        v4l2_fmtdesc format = {0};
        format.type = type;

        std::error_code query_error;

        for (format.index = 0;; ++format.index)
        {
            retry_ioctl(fd, VIDIOC_ENUM_FMT, &format, query_error);

            if (query_error)
                break;

            formats.push_back(format);
        }

        if (query_error == std::errc::invalid_argument)
        {
            // Is not actually an error - but the condition when there are
            // no more formats to enumerate.
            error.assign(0, std::generic_category());
        }
        else
        {
            error = query_error;
        }

        return formats;

    }

    /// Calls enumerate_formats(...) with a specific v4l2_buf_type and
    /// accumulates the results.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    auto formats = c4m::linux::enumerate_formats(fd);
    ///
    inline std::vector<v4l2_fmtdesc>
    enumerate_formats(const scoped_file_descriptor& fd, std::error_code& error)
    {
        assert(fd);

        // The supported buffer types for this operation:
        // http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-fmt.html
        std::vector<v4l2_buf_type> buffer_types = {
            V4L2_BUF_TYPE_VIDEO_CAPTURE, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
            V4L2_BUF_TYPE_VIDEO_OUTPUT, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
            V4L2_BUF_TYPE_VIDEO_OVERLAY };

        std::vector<v4l2_fmtdesc> formats;

        for (const auto& t : buffer_types)
        {
            auto new_formats = enumerate_formats(fd, t, error);

            if (error)
                return std::vector<v4l2_fmtdesc>();

            for(const auto& f : new_formats)
                formats.push_back(f);
        }

        return formats;
    }

    /// Calls enumerate_formats(...) with an error_code and throws an
    /// exception if an error is set.
    ///
    /// Example:
    ///
    ///    // Assuming fd is an open scoped_file_descriptor
    ///
    ///    auto formats = c4m::linux::enumerate_formats(fd);
    ///
    inline std::vector<v4l2_fmtdesc>
    enumerate_formats(const scoped_file_descriptor &fd)
    {
        assert(fd);

        std::error_code error;
        auto formats = enumerate_formats(fd, error);
        throw_if_error(error);
        return formats;
    }
}
}
