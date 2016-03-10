// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <cstdint>
#include <memory>
#include <libudev.h>

namespace c4m
{
namespace linux
{
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
            (void) error;
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
}
}
