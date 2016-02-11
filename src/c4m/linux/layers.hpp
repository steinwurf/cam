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
#include <sys/stat.h>
#include <libudev.h>

#include <libusb-1.0/libusb.h>

#include <memory>

namespace c4m
{
namespace linux
{
    /// Development docs:
    /// http://www.usb.org/developers/docs/devclass_docs/USB_Video_Class_1_5.zip
    ///
    /// The zip file contains a number of PDFs that we will reference in
    /// the documentation below.

    /// Constants needed
    /// Defined in:
    ///    UVC 1.5 Class specification.pdf
    ///
    /// Appendix A (p. 156)
    ///
    enum class usb_class_code
    {
        video = 0x0E
    };

    enum class usb_subclass_code
    {
        video_control = 0x01
    };

    enum class usb_class_specific_type
    {
        interface = 0x24
    };

    enum class usb_class_specific_subtype
    {
        extension_unit = 0x06
    };


    // Next up http://www.linux-usb.org/USB-guide/x75.html
    template<class Super>
    class create_udev_device : public Super
    {
    private:

        /// Small helper struct that is used as a custom deletor for the
        /// unique_ptr holding the pointer to udev. When the unique_ptr
        /// goes out of scope the operator gets invoked which then unref's
        /// the udev pointer.
        struct unreference
        {
            void operator()(udev* dev)
            {
                assert(dev);
                auto ok = udev_unref(dev);
                assert(ok == nullptr);
            }

            void operator()(udev_device* dev)
            {
                assert(dev);
                auto ok = udev_device_unref(dev);
                assert(ok == nullptr);
            }
        };


    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            auto udev_ptr = std::unique_ptr<udev, unreference>(udev_new());

            if (!udev_ptr)
            {
                assert(0); // <--- replace with error
                return;
            }

            // Get the device number
            struct stat stat_buffer;

            if (stat(device, &stat_buffer) != 0)
            {
                assert(0); // <--- replace with error
                return;
            }

            std::unique_ptr<udev_device, unreference> udev_device_ptr;

            if (S_ISBLK (stat_buffer.st_mode))
            {
                udev_device_ptr = std::unique_ptr<udev_device, unreference>(
                    udev_device_new_from_devnum(udev_ptr.get(), 'b',
                                                stat_buffer.st_rdev));
            }
            else if (S_ISCHR(stat_buffer.st_mode))
            {
                udev_device_ptr = std::unique_ptr<udev_device, unreference>(
                    udev_device_new_from_devnum(udev_ptr.get(), 'c',
                                                stat_buffer.st_rdev));
            }

            if (!udev_device_ptr)
            {
                assert(0);
                return;
            }

            // Life time of returned parent is tied to the m_udev_device
            // according to the documentation of the
            // udev_device_get_parent_xxxx(...) function
            udev_device* parent = udev_device_get_parent_with_subsystem_devtype(
                udev_device_ptr.get(), "usb", "usb_device");

            if (parent == nullptr)
            {
                assert(0);
                return;
            }

            uint64_t bus_number = get_sysattr_value(parent, "busnum", error);

            if (error)
                return;

            uint64_t dev_number = get_sysattr_value(parent, "devnum", error);

            if (error)
                return;

            std::cout << "bus = " << bus_number << " dev = " << dev_number
                      << std::endl;

            m_bus_number = bus_number;
            m_dev_number = dev_number;


        }

        uint64_t bus_number() const
        {
            return m_bus_number;
        }

        uint64_t dev_number() const
        {
            return dev_number;
        }

    private:

        uint64_t get_sysattr_value(udev_device* device, const char* name,
                                   std::error_code& error)
        {
            assert(device != nullptr);
            assert(name != nullptr);

            const char* value = udev_device_get_sysattr_value(
                device, name);

            if (value == nullptr)
            {
                assert(0);
                return 0;
            }

            return atol(value);
        }


    private:

        uint64_t m_bus_number;
        uint64_t m_dev_number;

    };


    template<class Super>
    class create_usb_device : public Super
    {
    private:

        /// Small helper struct that is used as a custom deletor for the
        /// unique_ptr holding the pointer to udev. When the unique_ptr
        /// goes out of scope the operator gets invoked which then unref's
        /// the udev pointer.
        struct unreference
        {
            void operator()(libusb_context* context)
            {
                assert(context);
                libusb_exit(context);
            }

        };

        struct scoped_device_list
        {
            scoped_device_list(libusb_device** device_list)
                : m_device_list(device_list)
            { }

            ~scoped_device_list()
            {
                if (m_device_list)
                {
                    // Free the list and unref all devices contained
                    libusb_free_device_list(m_device_list, 1);
                }
            }

            scoped_device_list(const scoped_device_list&) = delete;
            scoped_device_list& operator=(const scoped_device_list&) = delete;

            libusb_device** m_device_list = nullptr;
        };

    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if (error)
                return;

            libusb_context* context;
            if (libusb_init(&context) != 0)
            {
                assert(0);
                return;
            }

            // Pass ownership of the pointer to the unique_ptr
            auto context_ptr = std::unique_ptr<libusb_context, unreference>(
                context);

            libusb_device** device_list = nullptr;

            auto device_count = libusb_get_device_list(context, &device_list);
            if (device_count < 0)
            {
                assert(0);
                return;
            }

            scoped_device_list raii_device_list(device_list);

            // libusb_device* device;
            for (int i = 0; i < device_count; ++i)
            {
            }




        }
    };


    /// UVCX Video Config struct is used for probing and to config a
    /// specific configuration
    ///
    /// Defined in:
    ///    USB Device Class Definition for Video Devices: H.264 Payload.
    ///    Revision: 1.00
    ///    Page 11.
    ///    Link:
    ///
    struct uvcx_video_config
    {
        uint32_t m_frame_interval;
        uint32_t m_bitrate;
        uint16_t m_hints;
        uint16_t m_configuration_index;
        uint16_t m_width;
        uint16_t m_height;
        uint16_t m_slice_units;
        uint16_t m_slice_mode;
        uint16_t m_profile;
        uint16_t m_i_frame_period;
        uint16_t m_estimated_video_delay;
        uint16_t m_estimated_max_config_delay;
        uint8_t m_usage_type;
        uint8_t m_rate_control_mode;
        uint8_t m_temporal_scale_mode;
        uint8_t m_spatial_scale_mode;
        uint8_t m_snr_scale_mode;
        uint8_t m_stream_mux_option;
        uint8_t m_stream_format;
        uint8_t m_entropy_cabac;
        uint8_t m_timestamp;
        uint8_t m_num_of_reorder_frames;
        uint8_t m_preview_flipped;
        uint8_t m_view;
        uint8_t m_reserved1;
        uint8_t m_reserved2;
        uint8_t m_stream_id;
        uint8_t m_spatial_layer_ratio;
        uint16_t m_leaky_bucket_size;
    };

    // Output operator for the uvcx_video_config
    inline std::ostream&
    operator<<(std::ostream& os, const uvcx_video_config& config)
    {
        os << "c4m::linux::uvcx_video_config: "
           << "m_frame_interval = " << config.m_frame_interval << " "
           << "m_bitrate = " << config.m_bitrate << " "
           << "m_hints = " << config.m_hints << " "
           << "m_configuration_index = " << config.m_configuration_index << " "
           << "m_width = " << config.m_width << " "
           << "m_height = " << config.m_height << " "
           << "m_slice_units = " << config.m_slice_units << " "
           << "m_slice_mode = " << config.m_slice_mode << " "
           << "m_profile = " << config.m_profile << " "
           << "m_i_frame_period = " << config.m_i_frame_period << " "
           << "m_estimated_video_delay = "
           << config.m_estimated_video_delay << " "
           << "m_estimated_max_config_delay = "
           << config.m_estimated_max_config_delay << " "
           << "m_usage_type = " << config.m_usage_type << " ";

        return os;
    }


    template<class Super>
    class set_key_frame_interval : public Super
    {
    public:

        void start_streaming(std::error_code& error)
        {
            // https://cgit.freedesktop.org/gstreamer/gst-plugins-bad/tree/sys/uvch264/gstuvch264_src.c
            assert(!error);
            // uvcx_video_config_probe_commit_t probe;


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
