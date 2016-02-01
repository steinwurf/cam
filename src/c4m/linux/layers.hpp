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

namespace c4m
{
namespace linux
{

    template<class Super>
    class set_key_frame_interval : public Super
    {
    public:

        void start_streaming(std::error_code& error)
        {

            assert(!error);

            v4l2_ext_control extra_control;

            memset(&extra_control, 0, sizeof(extra_control));
            extra_control.id = V4L2_CID_MPEG_VIDEO_H264_I_PERIOD;
            extra_control.size = 0;
            extra_control.value = 10;

            v4l2_ext_controls extra_controls;
            memset(&extra_controls, 0, sizeof(extra_controls));

            extra_controls.controls = &extra_control;
            extra_controls.count = 1;
            extra_controls.ctrl_class = V4L2_CTRL_CLASS_MPEG;

            Super::retry_ioctl(VIDIOC_S_EXT_CTRLS, &extra_controls, error);

            if (error)
            {
                std::cout << "Damn" << std::endl;
                return;
            }


            Super::start_streaming(error);



// if (0 < ret)
// {
//     printf("VIDIOC_S_EXT_CTRLS setting (%s)\n", strerror(errno));
//     return -3;
// }/*if*/


// ret = ioctl(fd, VIDIOC_G_EXT_CTRLS, &extra_controls);
//  if (0 < ret)
// {
//     printf("VIDIOC_G_EXT_CTRLS setting (%s)\n", strerror(errno));
//     return -4;
// }/*if*/

// printf("extra_control.value = %d\n", extra_control.value );

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
