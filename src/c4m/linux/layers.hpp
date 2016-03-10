// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "../error_category.hpp"
#include "../is_error_code_enum.hpp"
#include "../make_error_code.hpp"
#include "../status_layer.hpp"
#include "../throw_if_error_layer.hpp"
#include "../trace_layer.hpp"
#include "../update_status_layer.hpp"
#include "async_capture_layer.hpp"
#include "buffer_queue_layer.hpp"
#include "capture_layer.hpp"
#include "check_has_streaming_io_ioctls_layer.hpp"
#include "check_supports_h264_format_layer.hpp"
#include "check_video_capture_capability_layer.hpp"
#include "create_udev_device.hpp"
#include "create_usb_device.hpp"
#include "enumerate_formats_layer.hpp"
#include "final_layer.hpp"
#include "ioctl_layer.hpp"
#include "memory_map_layer.hpp"
#include "open_layer.hpp"
#include "read_capability_layer.hpp"
#include "request_format_layer.hpp"
#include "retry_ioctl_layer.hpp"
#include "set_average_bitrate.hpp"
#include "set_h264_format_layer.hpp"
#include "streaming_layer.hpp"
#include "trace_buffer_queue_layer.hpp"
#include "trace_capture_layer.hpp"
#include "uvcx_video_config.hpp"
#include "xu_config_query.hpp"
#include "xu_query.hpp"
#include "zero_timestamp_at_initial_capture.hpp"

namespace c4m
{
namespace linux
{
    template<class Features>
    using camera =
        throw_if_error_layer<
        update_status_layer<
        set_average_bitrate<
        xu_config_query<
        xu_query<
        create_usb_device<
        create_udev_device<
        zero_timestamp_at_initial_capture<
        trace_capture_layer<find_enable_trace<Features>,
        async_capture_layer<
        capture_layer<
        streaming_layer<
        trace_buffer_queue_layer<find_enable_trace<Features>,
        buffer_queue_layer<
        memory_map_layer<
        set_h264_format_layer<
        check_supports_h264_format_layer<
        check_has_streaming_io_ioctls_layer<
        check_video_capture_capability_layer<
        request_format_layer<
        enumerate_formats_layer<
        read_capability_layer<
        retry_ioctl_layer<
        ioctl_layer<
        open_layer<
        trace_layer<find_enable_trace<Features>,
        status_layer<
        final_layer>>>>>>>>>>>>>>>>>>>>>>>>>>>;

}
}
