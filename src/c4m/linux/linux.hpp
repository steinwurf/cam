// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "scoped_file_descriptor.hpp"
#include "open.hpp"
#include "ioctl.hpp"
#include "read_capability.hpp"
#include "read_format.hpp"
#include "set_format.hpp"
#include "is_a_video_capture_device.hpp"
#include "has_streaming_io_ioctls.hpp"
#include "print_v4l2_format.hpp"
#include "request_memory_map_buffers.hpp"
#include "memory_map_buffers.hpp"
#include "enqueue_buffer.hpp"
#include "enqueue_buffers.hpp"
#include "start_streaming.hpp"
#include "dequeue_buffer.hpp"
#include "stop_streaming.hpp"
#include "enumerate_formats.hpp"
#include "v4l2_pixel_format_to_string.hpp"
#include "string_to_v4l2_pixel_format.hpp"
