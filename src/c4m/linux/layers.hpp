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

namespace c4m
{
namespace linux
{
    class final_layer
    { };

    template<class Super>
    class open_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            m_fd = c4m::linux::open(device, error);
        }

    protected:

        const scoped_file_descriptor& file_descriptor() const
        {
            return m_fd;
        }

    private:

        // The file descriptor associated with the device
        scoped_file_descriptor m_fd;

    };

    template<class Super>
    class read_capability_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            Super::open(device, error);

            if(error)
            {
                return;
            }

            const auto& fd = Super::file_descriptor();
            assert(fd);

            m_capability = {0};
            c4m::linux::read_capability(fd, &m_capability, error);
        }

        const v4l2_capability& capability() const
        {
            return m_capability;
        }

        bool is_a_video_capture_device() const
        {
            return c4m::linux::is_a_video_capture_device(m_capability);
        }

        bool has_streaming_io_ioctls() const
        {
            return c4m::linux::has_streaming_io_ioctls(m_capability);
        }

    private:

        v4l2_capability m_capability;

    };

    template<class Super>
    class enumerate_formats_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

            Super::open(device, error);

            if(error)
                return;

            const auto& fd = Super::file_descriptor();
            assert(fd);

            m_formats = c4m::linux::enumerate_formats(fd, error);
        }

        const std::vector<v4l2_fmtdesc>& formats() const
        {
            return m_formats;
        }

    private:

        std::vector<v4l2_fmtdesc> m_formats;

    };

    template<class Super>
    class check_video_capture_capability_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            Super::open(device, error);

            if(error)
                return;

            if (!Super::is_a_video_capture_device())
            {
                error = c4m::error::not_a_video_capture_device;
                return;
            }
        }
    };

    template<class Super>
    class check_has_streaming_io_ioctl_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            Super::open(device, error);

            if (error)
                return;

            if (!Super::has_streaming_io_ioctls())
            {
                error = c4m::error::does_not_have_streaming_io_ioctls;
                return;
            }
        }
    };

    template<class Super>
    class check_supports_h264_format_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            Super::open(device, error);

            if (error)
                return;

            auto formats = Super::formats();

            // If we find the H264 in the list of supported codecs we
            // return otherwise set the error code
            for(const auto& f : formats)
            {
                if (f.pixelformat == v4l2_fourcc('H','2','6','4'))
                    return;
            }

            error = c4m::error::does_not_support_h264_codec;

            // Super::set_fourcc("H264", error);

            // if (error)
            //     return;

            // // Check that we actually changed to H264
            // if (Super::fourcc() != "H264")
            //     error = c4m::error::format_is_not_h264;
        }

        // void request_format(uint32_t width, uint32_t height,
        //                     std::error_code& error)
        // {
        //     assert(width > 0);
        //     assert(height > 0);
        //     assert(!error);

        //     Super::request_format(width, height, error);

        //     if (error)
        //         return;

        //     // Check that we actually changed to H264
        //     if (Super::fourcc() != "H264")
        //         error = c4m::error::format_is_not_h264;
        // }
    };

    template<class Super>
    class request_format_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            Super::open(device, error);

            if (error)
                return;

            const auto& fd = Super::file_descriptor();
            assert(fd);

            m_format = c4m::linux::read_format(fd, error);
        }

        void request_resolution(uint32_t width, uint32_t height,
                                std::error_code& error)
        {
            m_format.fmt.pix.width = width;
            m_format.fmt.pix.height = height;

            const auto& fd = Super::file_descriptor();
            assert(fd);

            c4m::linux::set_format(fd, &m_format, error);
        }

        uint32_t width() const
        {
            return m_format.fmt.pix.width;
        }

        uint32_t height() const
        {
            return m_format.fmt.pix.height;
        }

        std::string fourcc() const
        {
            return v4l2_pixel_format_to_string(m_format.fmt.pix.pixelformat);
        }

    protected:

        void set_fourcc(const std::string& fourcc, std::error_code& error)
        {
            m_format.fmt.pix.pixelformat = string_to_v4l2_pixel_format(fourcc);

            const auto& fd = Super::file_descriptor();
            assert(fd);

            c4m::linux::set_format(fd, &m_format, error);
        }

    private:

        v4l2_format m_format;
    };


    template<class Super>
    class throw_if_error_layer : public Super
    {
    public:

        void open(const char* device)
        {
            std::error_code error;
            Super::open(device, error);

            throw_if_error(error);
        }
    };

    using camera =
        throw_if_error_layer<
        check_supports_h264_format_layer<
        check_has_streaming_io_ioctl_layer<
        check_video_capture_capability_layer<
        request_format_layer<
        enumerate_formats_layer<
        read_capability_layer<
        open_layer<
        final_layer>>>>>>>>;

    // template<class Super>
    // class read_frames
    // {

    //     template<class CompletionHandler>
    //     void read(const CompletionHandler& handler)
    //     {
    //         auto& fd = Super::file_descriptor();
    //         assert(fd);

    //         auto v4c4m::linux::dequeue_buffer(fd,


    //     }

    // };

    // /// Opens the webcam device that we will be using or sets the error_code
    // ///
    // /// Example:
    // ///
    // ///    std::error_code error;
    // ///    auto fd = c4m::linux::open("/dev/video1", &error);
    // ///
    // ///    if(error)
    // ///    {
    // ///        // Something when wrong
    // ///    }
    // ///
    // ///    // Device is open
    // ///
    // /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/func-open.html
    // ///
    // file_descriptor open(const char* device, std::error_code& error)
    // {
    //     assert(device);
    //     assert(error);

    //     file_descriptor fd(::open(device, O_RDWR));

    //     if (fd)
    //     {
    //         error->assign(0, std::generic_category());
    //     }
    //     else
    //     {
    //         error->assign(errno, std::generic_category());
    //     }

    //     return fd;
    // }

    // /// Opens the webcam device that we will be using or throws an exception
    // ///
    // /// Example:
    // ///
    // ///    auto fd = c4m::linux::open("/dev/video1");
    // ///
    // ///    // Device is open
    // ///
    // /// Docs: http://linuxtv.org/downloads/v4l-dvb-apis/func-open.html
    // ///
    // file_descriptor open(const char* device)
    // {
    //     std::error_code error;
    //     auto fd = open(device, &error);
    //     throw_if_error(error);
    //     return fd;
    // }
}
}
