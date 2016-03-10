// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "async_capture_layer.hpp"
#include "final_layer.hpp"
#include "open_layer.hpp"
#include "ioctl_layer.hpp"
#include "retry_ioctl_layer.hpp"
#include "read_capability_layer.hpp"
#include "enumerate_formats_layer.hpp"
#include "request_format_layer.hpp"
#include "check_video_capture_capability_layer.hpp"
#include "check_has_streaming_io_ioctls_layer.hpp"
#include "check_supports_h264_format_layer.hpp"
#include "set_h264_format_layer.hpp"
#include "memory_map_layer.hpp"
#include "buffer_queue_layer.hpp"
#include "streaming_layer.hpp"
#include "capture_layer.hpp"
#include "zero_timestamp_at_initial_capture.hpp"

#include "scoped_file_descriptor.hpp"

#include "ioctl.hpp"

#include "has_streaming_io_ioctls.hpp"
#include "print_v4l2_format.hpp"

#include "v4l2_pixelformat_to_string.hpp"
#include "string_to_v4l2_pixelformat.hpp"
