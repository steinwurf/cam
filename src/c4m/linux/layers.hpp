// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "linux.hpp"
#include "../error.hpp"
#include "../is_error_code_enum.hpp"
#include "../error_category.hpp"
#include "../make_error_code.hpp"
#include "../trace_layer.hpp"
#include "trace_capture_layer.hpp"

#include <linux/uvcvideo.h>
#include <linux/usb/video.h>

namespace c4m
{
namespace linux
{

    struct uvcx_video_config
    {
    };


    template<class Super>
    class set_key_frame_interval : public Super
    {
    public:

        void start_streaming(std::error_code& error)
        {
            // https://cgit.freedesktop.org/gstreamer/gst-plugins-bad/tree/sys/uvch264/gstuvch264_src.c
            assert(!error);
            uvcx_video_config_probe_commit_t probe;


        }



    };

    template<class Features>
    using camera2 =
        throw_if_error_layer<
        set_key_frame_interval<
        zero_timestamp_at_initial_capture<
        trace_capture_layer<find_enable_trace<Features>,
        capture_layer<
        streaming_layer<
        buffer_queue_layer<
        memory_map_layer<
        set_h264_format_layer<
        check_supports_h264_format_layer<
        check_has_streaming_io_ioctl_layer<
        check_video_capture_capability_layer<
        request_format_layer2<
        enumerate_formats_layer2<
        read_capability_layer2<
        retry_ioctl_layer<
        ioctl_layer<
        open_layer2<
        trace_layer<find_enable_trace<Features>,
        final_layer2>>>>>>>>>>>>>>>>>>>;

}
}
