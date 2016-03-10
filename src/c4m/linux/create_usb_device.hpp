// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "extension_unit_descriptor.hpp"

#include <libusb-1.0/libusb.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <system_error>

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
            (void) error;
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

            libusb_device_descriptor descriptor;

            if (libusb_get_device_descriptor(
                    usb_device.get(), &descriptor) != 0)
            {
                assert(0);
                return;
            }


            for (uint32_t i = 0; i < descriptor.bNumConfigurations; ++i)
            {
                auto config =
                    std::unique_ptr<libusb_config_descriptor, unreference>(
                        get_config_descriptor(usb_device.get(), i, error));

                if (error)
                    return;

                assert(config);

                for (uint8_t j = 0; j < config->bNumInterfaces; ++j)
                {
                    for (int32_t k = 0;
                         k < config->interface[j].num_altsetting; ++k)
                    {
                        const libusb_interface_descriptor* interface;
                        interface = &config->interface[j].altsetting[k];

                        if (interface->bInterfaceClass !=
                            (int)usb_class_code::video)
                            continue;

                        if (interface->bInterfaceSubClass !=
                            (int)usb_subclass_code::video_control)
                            continue;



                        const uint8_t* ptr = interface->extra;

                        while(ptr - interface->extra +
                              sizeof(extension_unit_descriptor) <
                              (uint32_t)interface->extra_length)
                        {

                            auto* desc = (extension_unit_descriptor*) ptr;

                            if (desc->m_descriptor_type ==
                                (int8_t)
                                usb_class_specific_type::interface &&
                                desc->m_descriptor_sub_type ==
                                (int8_t)
                                usb_class_specific_subtype::extension_unit)
                            {



                                if (memcmp(desc->m_guid_extension_code,
                                           guid, 15) == 0)
                                {
                                    m_unit_id = desc->m_unit_id;
                                }

                            }

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
}
}
