// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

C4M_ERROR_TAG(unused,
              "reserved since errors should never be zero :)")

C4M_ERROR_TAG(not_a_video_capture_device,
              "The device is not a video capture device")

C4M_ERROR_TAG(does_not_have_streaming_io_ioctls,
              "The device does not have the streaming io ioctls")

C4M_ERROR_TAG(does_not_support_h264_codec,
              "The device does not support the H264 codec")

C4M_ERROR_TAG(format_is_not_h264,
              "The device format is currently not H264")

C4M_ERROR_TAG(unable_to_allocate_buffers,
              "Unable to allocate memory mapped buffers - device may be "
              "out of memory")

C4M_ERROR_TAG(could_not_set_bitrates, "Unable to set bitrates")
