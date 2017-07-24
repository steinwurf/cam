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
#include <sys/stat.h>

namespace cam
{
namespace linux
{
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
