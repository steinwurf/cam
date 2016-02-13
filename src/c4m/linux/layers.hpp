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
    typedef struct
    {
        int8_t bLength;
        int8_t bDescriptorType;
        int8_t bDescriptorSubType;
        int8_t bUnitID;
        uint8_t guidExtensionCode[16];
    } __attribute__ ((__packed__)) xu_descriptor;

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

            // libusb_device* device;
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
                              sizeof(xu_descriptor) < interface->extra_length)
                        {

                            xu_descriptor *desc = (xu_descriptor *) ptr;

                            if (desc->bDescriptorType ==
                                (int8_t) usb_class_specific_type::interface &&
                                desc->bDescriptorSubType ==
                                (int8_t) usb_class_specific_subtype::extension_unit)
                            {

                                uint8_t guid[] = {0x41, 0x76, 0x9e, 0xa2, 0x04, 0xde, 0xe3, 0x47, 0x8b, 0x2b, 0xF4, 0x34, 0x1A, 0xFF, 0x00, 0x3B};

                                if (memcmp(desc->guidExtensionCode, guid, 15) == 0)
                                {
                                    std::cout << "Foudn it = "
                                              << (uint32_t) desc->bUnitID << std::endl;
                                    m_unit_id = desc->bUnitID;
                                }
                                else
                                {
                                    std::cout << "Close but no cigar" << std::endl;
                                }


                            }

                            std::cout << "Getting warmer" << std::endl;
                            ptr += desc->bLength;
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
    public:

        void config_query(std::error_code& error)
        {
            uvcx_video_config config;
            memset(&config, 0, sizeof(config));

            Super::query(0x01, UVC_GET_CUR,
                         (uint8_t*) &config, error);

            if (error)
            {
                std::cout << "FUCK FUCK FUCK" << std::endl;
                assert(0);
                return;
            }

            std::cout << config << std::endl;

            config.m_i_frame_period = 1000;
            config.m_width = 800;
            config.m_height = 600;

            Super::query(0x01, UVC_SET_CUR,
                         (uint8_t*) &config, error);

            if (error)
            {
                std::cout << "FUCK FUCK FUCK 2 " << std::endl;
                assert(0);
                return;
            }

            Super::query(0x01, UVC_GET_CUR,
                         (uint8_t*) &config, error);

            if (error)
            {
                std::cout << "FUCK FUCK FUCK 3" << std::endl;
                assert(0);
                return;
            }

            std::cout << config << std::endl;

            Super::query(0x02, UVC_SET_CUR, (uint8_t*) &config, error);

            if (error)
            {
                std::cout << "FUCK FUCK FUCK 4" << std::endl;
                assert(0);
                return;
            }
        }
    };

    typedef struct _uvcx_bitrate_layers_t
    {
        uint16_t wLayerID;
        uint32_t dwPeakBitrate;
        uint32_t dwAverageBitrate;
    } __attribute__((packed)) uvcx_bitrate_layers_t;

    // Output operator for the uvcx_video_config
    inline std::ostream&
    operator<<(std::ostream& os, const uvcx_bitrate_layers_t& bitrates)
    {
        os << "c4m::linux::uvcx_bitrate_layers_t: "
           << "wLayerID = " << bitrates.wLayerID << " "
           << "dwPeakBitrate = " << bitrates.dwPeakBitrate << " "
           << "dwAverageBitrate = " << bitrates.dwAverageBitrate << " ";

        return os;
    }

    template<class Super>
    class set_average_bitrate : public Super
    {
    public:
        void set_bitrates(uint32_t average_bitrate, uint32_t peak_bitrate,
                         std::error_code& error)
        {
             uvcx_bitrate_layers_t bitrates;
             memset(&bitrates, 0, sizeof(bitrates));

             Super::query(0x0E, UVC_GET_CUR, (uint8_t*) &bitrates, error);

             if (error)
             {
                 std::cout << "YUCK YUCK" << std::endl;
                 assert(0);
                 return;
             }

             std::cout << bitrates << std::endl;

             bitrates.dwPeakBitrate = peak_bitrate;
             bitrates.dwAverageBitrate = average_bitrate;

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
            final_layer2>>>>>>>>>>>>>>>>>>>>>>>>;

}
}
