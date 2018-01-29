// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "../is_error_code_enum.hpp"
#include "../throw_if_error_layer.hpp"
#include "../update_status_layer.hpp"
#include "set_average_bitrate.hpp"
#include "xu_config_query.hpp"
#include "xu_query_layer.hpp"
#include "create_usb_device.hpp"
#include "create_udev_device.hpp"
#include "zero_timestamp_at_initial_capture.hpp"
#include "trace_capture_layer.hpp"
#include "async_capture_layer.hpp"
#include "capture_layer.hpp"
#include "streaming_layer.hpp"
#include "trace_buffer_queue_layer.hpp"
#include "buffer_queue_layer.hpp"
#include "memory_map_layer.hpp"
#include "set_h264_format_layer.hpp"
#include "check_supports_h264_format_layer.hpp"
#include "check_has_streaming_io_ioctls_layer.hpp"
#include "check_video_capture_capability_layer.hpp"
#include "request_format_layer.hpp"
#include "enumerate_formats_layer.hpp"
#include "read_capability_layer.hpp"
#include "retry_ioctl_layer.hpp"
#include "ioctl_layer.hpp"
#include "trace_open_layer.hpp"
#include "open_layer.hpp"
#include "../trace_layer.hpp"
#include "../status_layer.hpp"
#include "final_layer.hpp"
#include "trace_request_format_layer.hpp"
#include "trace_create_usb_device.hpp"
#include "trace_set_average_bitrate.hpp"
#include "trace_xu_config_query.hpp"
#include "trace_xu_query_layer.hpp"
#include "trace_ioctl_layer.hpp"

namespace cam
{
namespace linux
{
    using camera =
        throw_if_error_layer<
        update_status_layer<
        //trace_set_average_bitrate<
        //set_average_bitrate<
        //trace_xu_config_query<
        //xu_config_query<
        trace_create_usb_device<
        create_usb_device<
        create_udev_device<
        trace_xu_query_layer<
        xu_query_layer<
        zero_timestamp_at_initial_capture<
        trace_capture_layer<
        async_capture_layer<
        capture_layer<
        streaming_layer<
        trace_buffer_queue_layer<
        buffer_queue_layer<
        memory_map_layer<
        set_h264_format_layer<
        check_supports_h264_format_layer<
        check_has_streaming_io_ioctls_layer<
        check_video_capture_capability_layer<
        trace_request_format_layer<
        request_format_layer<
        enumerate_formats_layer<
        read_capability_layer<
        retry_ioctl_layer<
        trace_ioctl_layer<
        ioctl_layer<
        trace_open_layer<
        open_layer<
        trace_layer<
        status_layer<
        final_layer>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>;//>>>>;
}
}
