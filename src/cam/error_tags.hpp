// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

CAM_ERROR_TAG(unused,
              "reserved since errors should never be zero :)")

CAM_ERROR_TAG(not_a_video_capture_device,
              "The device is not a video capture device")

CAM_ERROR_TAG(does_not_have_streaming_io_ioctls,
              "The device does not have the streaming io ioctls")

CAM_ERROR_TAG(does_not_support_h264_codec,
              "The device does not support the H264 codec")

CAM_ERROR_TAG(format_is_not_h264,
              "The device format is currently not H264")

CAM_ERROR_TAG(unable_to_allocate_buffers,
              "Unable to allocate memory mapped buffers - device may be "
              "out of memory")

CAM_ERROR_TAG(could_not_set_bitrates, "Unable to set bitrates")
