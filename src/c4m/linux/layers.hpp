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

namespace c4m
{
namespace linux
{

    using camera2 =
        throw_if_error_layer<
        zero_timestamp_at_initial_capture<
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
        final_layer2>>>>>>>>>>>>>>>>;

}
}
