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
#include "../status_layer.hpp"
#include "../update_status_layer.hpp"
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
    /// The extension unit descriptor.
    ///
    /// Defined in:
    ///    UVC 1.5 Class specification.pdf
    ///
    /// Section 3.7.2.7 (p. 58)
    ///
    struct extension_unit_descriptor
    {
        // Size of this descriptor in bytes
        int8_t m_length;

        /// Constant of CS_INTERFACE descriptor type
        int8_t m_descriptor_type;

        /// Constant of VC_EXTENSION_UNIT descriptor subtype
        int8_t m_descriptor_sub_type;

        // Unique identifier for unit, this value must be passed with each
        // request that is directed to the Extension Unit.
        int8_t m_unit_id;

        // Vendor specific code that further identifies the extension unit.
        uint8_t m_guid_extension_code[16];
    } __attribute__ ((__packed__));

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

    /// Globally Unique Identifier (GUID) for the extension code.
    ///
    /// Note: The layout of the GUID is specified in USB_Video_FAQ_1.5.pdf
    ///       (section "Layout of a GUID Data Structure" p. 15)
    ///
    /// Layout (and example):
    ///
    /// +-----------------------------------+
    /// |offset|Field |Size (bytes)|Example |
    /// +-----------------------------------+
    /// |0     |Data1 |4           |47504a4d|
    /// +-----------------------------------+
    /// |4     |Data2 |2           |0000    |
    /// +-----------------------------------+
    /// |6     |Data3 |2           |0010    |
    /// +-----------------------------------+
    /// |8     |Data4a|1           |80      |
    /// +-----------------------------------+
    /// |9     |Data4b|1           |00      |
    /// +-----------------------------------+
    /// |10    |Data4c|1           |00      |
    /// +-----------------------------------+
    /// |11    |Data4d|1           |aa      |
    /// +-----------------------------------+
    /// |12    |Data4e|1           |00      |
    /// +-----------------------------------+
    /// |13    |Data4f|1           |38      |
    /// +-----------------------------------+
    /// |14    |Data4g|1           |9b      |
    /// +-----------------------------------+
    /// |15    |Data4h|1           |71      |
    /// +-----------------------------------+
    ///
    /// In the example given in the table above the GUID
    /// {47504a4d-0000-0010-80-00-00-aa-00-38-9b-71} will have the
    /// following little-endian byte-stream representation:
    ///
    /// {0x4d,0x4a,0x50,0x47,0x00,0x00,0x10,0x00,
    ///  0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71}
    ///
    /// lsusb -d 046d:082d -d to get a list of XU
    ///
    /// The following is defined in:
    ///     USB Device Class Definition for Video Devices: H.264 Payload.
    ///     Revision: 1.00
    ///     Appendix A (p. 43)
    ///
    static const uint8_t guid[] = {0x41, 0x76, 0x9e, 0xa2,
                                   0x04, 0xde, 0xe3, 0x47,
                                   0x8b, 0x2b, 0xF4, 0x34,
                                   0x1A, 0xFF, 0x00, 0x3B};

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

            Super::open(device, error);

            if (error)
                return;

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
            return m_dev_number;
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

            void operator()(libusb_device* device)
            {
                assert(device);
                libusb_unref_device(device);
            }

            void operator()(libusb_config_descriptor* config)
            {
                assert(config);
                libusb_free_config_descriptor(config);
            }

            void operator()(libusb_device** device_list)
            {
                assert(device_list);

                // Free the list and unref all devices contained
                libusb_free_device_list(device_list, 1);

            }


        };


        libusb_context* get_context(std::error_code& error)
        {
            assert(!error);
            libusb_context* context;

            if (libusb_init(&context) != 0)
            {
                assert(0);
                return nullptr;
            }
            return context;
        }

        libusb_device** get_device_list(libusb_context* context,
                                        ssize_t& device_count,
                                        std::error_code& error)
        {
            libusb_device** device_list = nullptr;

            device_count = libusb_get_device_list(context, &device_list);
            if (device_count < 0)
            {
                assert(0);
                return nullptr;
            }

            return device_list;
        }


        libusb_config_descriptor* get_config_descriptor(
            libusb_device* device, uint8_t index, std::error_code& error)
        {
            assert(device);
            assert(!error);

            libusb_config_descriptor* config;
            if (libusb_get_config_descriptor(device, index, &config) != 0)
            {
                assert(0);
                assert(config == nullptr);
                return nullptr;
            }

            return config;
        }

        // uint8_t find_unit_in_configurations(libusb_device_descriptor& device,
        //                                     std::error_code& error)
        // {
        //     using unique_config =
        //         std::unique_ptr<libusb_config_descriptor, unreference>;

        //     for (uint32_t i = 0; i < descriptor.bNumConfigurations; ++i)
        //     {
        //         auto config = unique_config(
        //             get_config_descriptor(usb_device.get(), i, error));

        //         if (error)
        //             return;

        //         assert(config);

        //         find_unit_in_interfaces

        // }

    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if (error)
                return;

            auto context = std::unique_ptr<libusb_context, unreference>(
                get_context(error));

            if (error)
                return;

            assert(context);

            ssize_t device_count = 0;

            auto device_list = std::unique_ptr<libusb_device*, unreference>(
                get_device_list(context.get(), device_count, error));

            if (error)
                return;

            assert(device_list);

            std::unique_ptr<libusb_device, unreference> usb_device;

            for (int i = 0; i < device_count; ++i)
            {
                auto d = device_list.get()[i];
                assert(d);

                if (Super::bus_number() != libusb_get_bus_number(d))
                    continue;

                if (Super::dev_number() != libusb_get_device_address(d))
                    continue;

                usb_device = std::unique_ptr<libusb_device, unreference>(
                    libusb_ref_device(d));

                assert(usb_device);
                break;
            }

            if (!usb_device)
            {
                assert(0);
                return;
            }

            std::cout << "Device found" << std::endl;

            libusb_device_descriptor descriptor;

            if (libusb_get_device_descriptor(
                    usb_device.get(), &descriptor) != 0)
            {
                assert(0);
                return;
            }

            std::cout << "Descriptor" << std::endl;

            for (uint32_t i = 0; i < descriptor.bNumConfigurations; ++i)
            {
                auto config = std::unique_ptr<libusb_config_descriptor, unreference>(
                    get_config_descriptor(usb_device.get(), i, error));

                if (error)
                    return;

                assert(config);

                for (uint32_t j = 0; j < config->bNumInterfaces; ++j)
                {
                    for (uint32_t k = 0; k < config->interface[j].num_altsetting; ++k)
                    {
                        const libusb_interface_descriptor* interface;
                        interface = &config->interface[j].altsetting[k];

                        if (interface->bInterfaceClass != (int)usb_class_code::video)
                            continue;

                        if (interface->bInterfaceSubClass !=
                            (int)usb_subclass_code::video_control)
                            continue;

                        std::cout << "So far so good" << std::endl;

                        const uint8_t* ptr = interface->extra;

                        while(ptr - interface->extra +
                              sizeof(extension_unit_descriptor) < interface->extra_length)
                        {

                            auto* desc = (extension_unit_descriptor*) ptr;

                            if (desc->m_descriptor_type ==
                                (int8_t) usb_class_specific_type::interface &&
                                desc->m_descriptor_sub_type ==
                                (int8_t) usb_class_specific_subtype::extension_unit)
                            {



                                if (memcmp(desc->m_guid_extension_code, guid, 15) == 0)
                                {
                                    std::cout << "Foudn it = "
                                              << (uint32_t) desc->m_unit_id << std::endl;
                                    m_unit_id = desc->m_unit_id;
                                }
                                else
                                {
                                    std::cout << "Close but no cigar" << std::endl;
                                }


                            }

                            std::cout << "Getting warmer" << std::endl;
                            ptr += desc->m_length;
                        }
                    }

                }
            }
        }


        uint8_t unit_id() const
        {
            return m_unit_id;
        }

    private:

        uint8_t m_unit_id;

    };


    template<class Super>
    class xu_query : public Super
    {
    public:

        void query(uint8_t selector, uint8_t query, uint8_t* data,
                   std::error_code& error)
        {
            assert(data);
            assert(!error);

            uvc_xu_control_query xu;
            memset(&xu, 0, sizeof(xu));

            uint16_t length = 0;

            xu.unit = Super::unit_id();
            xu.selector = selector;
            xu.query = UVC_GET_LEN;
            xu.size = sizeof(length);
            xu.data = (uint8_t*) &length;

            Super::retry_ioctl(UVCIOC_CTRL_QUERY, &xu, error);

            if (error)
                return;

            if (query == UVC_GET_LEN)
            {
                *((uint16_t*)data) = length;
                return;
            }

            xu.query = query;
            xu.size = length;
            xu.data = data;

            Super::retry_ioctl(UVCIOC_CTRL_QUERY, &xu, error);
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
    } __attribute__((packed));

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
    class xu_config_query : public Super
    {
    private:

        /// The following is defined in:
        ///    USB Device Class Definition for Video Devices: H.264 Payload.
        ///    Revision: 1.00
        ///    Section "3.3 H.264 UVC Extensions Units (XUs)" (p. 11)
        ///
        enum class selector
        {
            uvcx_video_config_probe = 0x01,
            uvcx_video_config_commit = 0x02
        };

    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if (error)
                return;

            memset(&m_config, 0, sizeof(m_config));

            Super::query(
                static_cast<uint8_t>(selector::uvcx_video_config_probe),
                UVC_GET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);
        }


        void start_streaming(std::error_code& error)
        {
            assert(!error);

            // No buffers should be allocated yet, we need to configure the
            // encoder before we map memory etc. It is not a fact, but
            // seems reasonable since the size of memory will depend on the
            // chosen resolution etc.
            assert(Super::buffer_count() == 0);

            Super::query(
                static_cast<uint8_t>(selector::uvcx_video_config_commit),
                UVC_SET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);

            if (error)
                return;

            Super::start_streaming(error);
        }

        // uint32_t width() const
        // {
        //     return m_config.m_width;
        // }

        // uint32_t height() const
        // {
        //     return m_config.m_height;
        // }

        // void request_resolution(uint32_t width, uint32_t height,
        //                         std::error_code& error)
        // {
        //     assert(!error);
        //     assert(width > 0);
        //     assert(height > 0);

        //     std::cout << "request res" << std::endl;

        //     m_config.m_width = width;
        //     m_config.m_height = height;

        //     Super::query(
        //         static_cast<uint8_t>(selector::uvcx_video_config_probe),
        //         UVC_SET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);
        // }

        uint32_t i_frame_period() const
        {
            return m_config.m_i_frame_period;
        }

        void request_i_frame_period(uint32_t i_frame_period,
                                    std::error_code& error)
        {
            assert(!error);

            std::cout << "request iframe" << std::endl;

            m_config.m_i_frame_period = i_frame_period;

            Super::query(
                static_cast<uint8_t>(selector::uvcx_video_config_probe),
                UVC_SET_CUR, reinterpret_cast<uint8_t*>(&m_config), error);
        }

    private:

        uvcx_video_config m_config;
    };

    struct uvcx_bitrate
    {
        uint16_t m_layer_id;
        uint32_t m_peak_bitrate;
        uint32_t m_average_bitrate;
    } __attribute__((packed));

    // Output operator for the uvcx_video_config
    inline std::ostream&
    operator<<(std::ostream& os, const uvcx_bitrate& bitrates)
    {
        os << "c4m::linux::uvcx_bitrate: "
           << "m_layer_id = " << bitrates.m_layer_id << " "
           << "m_peak_bitrate = " << bitrates.m_peak_bitrate << " "
           << "m_average_bitrate = " << bitrates.m_average_bitrate << " ";

        return os;
    }

    template<class Super>
    class set_average_bitrate : public Super
    {
    public:

        void request_bitrates(uint32_t average_bitrate, uint32_t peak_bitrate,
                              std::error_code& error)
        {
             uvcx_bitrate bitrates;
             memset(&bitrates, 0, sizeof(bitrates));

             Super::query(0x0E, UVC_GET_CUR, (uint8_t*) &bitrates, error);

             if (error)
             {
                 std::cout << "YUCK YUCK" << std::endl;
                 assert(0);
                 return;
             }

             std::cout << bitrates << std::endl;

             bitrates.m_peak_bitrate = peak_bitrate;
             bitrates.m_average_bitrate = average_bitrate;

             Super::query(0x0E, UVC_SET_CUR, (uint8_t*) &bitrates, error);

             if (error)
             {
                 std::cout << "YUCK YUCK 2" << std::endl;
                 assert(0);
                 return;
             }

             std::cout << bitrates << std::endl;
        }
    };


    /// Fall-through case for the case where TraceTag is meta::not_found
    template<class TraceTag, class Super>
    class trace_buffer_queue_layer : public Super
    {
        static_assert(std::is_same<TraceTag, meta::not_found>::value,
                      "Unexpected TraceTag should be meta::not_found in the "
                      "fall-through case.");
    };

    template<class Super>
    class trace_buffer_queue_layer<enable_trace, Super> : public Super
    {
    public:

        /// Called to initialize memory
        void start_streaming(std::error_code& error)
        {
            assert(!error);

            if (Super::is_trace_enabled())
            {
                Super::write_trace("buffer_queue_layer", "start_streaming");
            }

            Super::start_streaming(error);
        }

        void stop_streaming(std::error_code& error)
        {
            assert(!error);

            if (Super::is_trace_enabled())
            {
                Super::write_trace("buffer_queue_layer", "stop_streaming");
            }

            Super::stop_streaming(error);
        }
        void enqueue_buffer(uint32_t index, std::error_code& error)
        {
            assert(!error);

            if (Super::is_trace_enabled())
            {
                std::stringstream ss;
                ss << "enqueue_buffer: index=" << index;
                Super::write_trace(
                    "buffer_queue_layer", ss.str());
            }

            Super::enqueue_buffer(index, error);
        }

        v4l2_buffer dequeue_buffer(std::error_code& error)
        {
            assert(!error);
            auto buffer = Super::dequeue_buffer(error);

            if (Super::is_trace_enabled())
            {
                assert(!error);
                std::stringstream ss;
                ss << "dequeu_buffer: index=" << buffer.index;
                Super::write_trace(
                    "buffer_queue_layer", ss.str());
            }
            return buffer;
        }
    };




    template<class Features>
    using camera2 =
        throw_if_error_layer<
        update_status_layer<
        set_average_bitrate<
        xu_config_query<
        xu_query<
        create_usb_device<
        create_udev_device<
        zero_timestamp_at_initial_capture<
        trace_capture_layer<find_enable_trace<Features>,
        capture_layer<
        streaming_layer<
        trace_buffer_queue_layer<find_enable_trace<Features>,
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
        status_layer<
            final_layer2>>>>>>>>>>>>>>>>>>>>>>>>>>;

}
}
