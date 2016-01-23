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

    using camera2 =
        throw_if_error_layer<
        zero_timestamp_at_initial_capture<
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
        final_layer2>>>>>>>>>>>>>>>>;

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

            memset(&m_capability, 0, sizeof(m_capability));
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
    class request_format_layer : public Super
    {
    public:

        void open(const char* device, std::error_code& error)
        {
            assert(device);
            assert(!error);

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
            assert(width > 0);
            assert(height > 0);
            assert(!error);

            m_format.fmt.pix.width = width;
            m_format.fmt.pix.height = height;

            const auto& fd = Super::file_descriptor();
            assert(fd);

            c4m::linux::set_format(fd, &m_format, error);
        }

        void request_pixelformat(const std::string& fourcc,
                                 std::error_code& error)
        {
            assert(fourcc.size() == 4);
            assert(!error);

            m_format.fmt.pix.pixelformat = string_to_v4l2_pixelformat(fourcc);

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

        std::string pixelformat() const
        {
            return v4l2_pixelformat_to_string(m_format.fmt.pix.pixelformat);
        }

    private:

        v4l2_format m_format;
    };



    // template<class Super>
    // class start_streaming_layer : public Super
    // {
    // public:

    //     start_streaming_layer() //: m_initial_timestamp(0)
    //     { }

    //     void start_streaming(std::error_code& error)
    //     {
    //         assert(!error);

    //         const auto& fd = Super::file_descriptor();
    //         assert(fd);

    //         // We ask for 3 buffers would be nice to make this a parameters
    //         // at some point
    //         uint32_t buffer_count =
    //             c4m::linux::request_memory_map_buffers(fd, 3, error);

    //         if (error)
    //             return;

    //         // Driver may be out of memory:
    //         // https://linuxtv.org/downloads/v4l-dvb-apis/vidioc-reqbufs.html
    //         if (buffer_count == 0)
    //         {
    //             error = c4m::error::unable_to_allocate_buffers;
    //             return;
    //         }

    //         m_buffers =
    //             c4m::linux::memory_map_buffers(fd, buffer_count, error);

    //         if (error)
    //             return;

    //         assert(m_buffers.size() > 0);
    //         assert(m_buffers.size() == buffer_count);

    //         c4m::linux::enqueue_buffers(fd, buffer_count, error);

    //         if (error)
    //             return;

    //         c4m::linux::start_streaming(fd, error);

    //     }

    //     uint32_t max_buffer_size() const
    //     {
    //         assert(m_buffers.size() > 0);
    //         uint32_t max_size = m_buffers[0].size();
    //         assert(max_size > 0);

    //         // A bit of paranoia
    //         for(const auto& b : m_buffers)
    //         {
    //             assert(b);
    //             assert(b.size() == max_size);
    //         }

    //         return max_size;
    //     }

    //     uint64_t timestamp_to_micro_seconds(const v4l2_buffer& buffer)
    //     {

    //         auto clock_type = buffer.flags & V4L2_BUF_FLAG_TIMESTAMP_MASK;
    //         assert(clock_type == V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC);

    //         uint64_t time = (buffer.timestamp.tv_sec * 1000000) +
    //             buffer.timestamp.tv_usec;

    //         // @todo if clock type is V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN
    //         // we can do the timestamp ourselves with some chrono or such

    //         return time;
    //     }

    //     capture_data capture(std::error_code& error)
    //     {
    //         const auto& fd = Super::file_descriptor();
    //     //     assert(fd);

    //     }

    //     // void fill_buffer(uint8_t* data, std::error_code& error)
    //     // {
    //     //     assert(data);

    //     //     const auto& fd = Super::file_descriptor();
    //     //     assert(fd);

    //     //     auto buffer_info = c4m::linux::dequeue_buffer(fd, error);

    //     //     if (error)
    //     //         return;

    //     //     uint32_t index = buffer_info.index;

    //     //     m_bytesused = buffer_info.bytesused;
    //     //     m_timestamp = timestamp_to_micro_seconds(buffer_info);

    //     //     assert(index < buffers.size());

    //     //     const auto& b = buffers[index];

    //     // }



    // private:

    //     std::vector<memory_map_buffer> m_buffers;

    // };

    template<class Super>
    class start_streaming_layer : public Super
    {
    public:

        // start_streaming_layer() :
        //     m_initial_capture(true),
        //     m_dequeued_index(0)
        // { }

        // capture_data capture(std::error_code& error)
        // {
        //     assert(!error);

        //     const auto& fd = Super::file_descriptor();
        //     assert(fd);

        //     if (m_initial_capture)
        //     {
        //         // If this is the initial capture we enqueue all buffers
        //         c4m::linux::enqueue_buffers(fd, Super::buffer_count(), error);

        //         if (error)
        //             return capture_data();

        //         c4m::linux::start_streaming(fd, error);

        //         if (error)
        //             return capture_data();
        //     }
        //     else
        //     {
        //         // We re-enqueue the previously dequeued buffer
        //         c4m::linux::enqueue_buffer(fd, m_dequeued_index, error);

        //         if (error)
        //             return capture_data();
        //     }

        //     // This call will block
        //     auto buffer_info = c4m::linux::dequeue_buffer(fd, error);

        //     if (error)
        //         return capture_data();

        //     uint32_t index = buffer_info.index;
        //     uint32_t bytesused = buffer_info.bytesused;
        //     uint64_t timestamp = timestamp_to_micro_seconds(buffer_info);

        //     const auto& buffer = Super::buffer(index);

        //     capture_data data;
        //     data.m_data = buffer.data();
        //     data.m_size = bytesused;
        //     data.m_timestamp = timestamp;

        //     return data;
        // }



        uint64_t timestamp_to_micro_seconds(const v4l2_buffer& buffer)
        {

            auto clock_type = buffer.flags & V4L2_BUF_FLAG_TIMESTAMP_MASK;
            assert(clock_type == V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC);

            uint64_t time = (buffer.timestamp.tv_sec * 1000000) +
                buffer.timestamp.tv_usec;

            // @todo if clock type is V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN
            // we can do the timestamp ourselves with some chrono or such

            return time;
        }

        // capture_data capture(std::error_code& error)
        // {
        //     const auto& fd = Super::file_descriptor();
        // //     assert(fd);

        // }

        // void fill_buffer(uint8_t* data, std::error_code& error)
        // {
        //     assert(data);

        //     const auto& fd = Super::file_descriptor();
        //     assert(fd);

        //     auto buffer_info = c4m::linux::dequeue_buffer(fd, error);

        //     if (error)
        //         return;

        //     uint32_t index = buffer_info.index;

        //     m_bytesused = buffer_info.bytesused;
        //     m_timestamp = timestamp_to_micro_seconds(buffer_info);

        //     assert(index < buffers.size());

        //     const auto& b = buffers[index];

        // }



    private:

        std::vector<memory_map_buffer> m_buffers;

    };


    template<class Super>
    class throw_if_error_layer : public Super
    {
    public:

        void open(const char* device)
        {
            assert(device);

            std::error_code error;
            Super::open(device, error);

            throw_if_error(error);
        }

        void request_resolution(uint32_t width, uint32_t height)
        {
            assert(width > 0);
            assert(height > 0);

            std::error_code error;
            Super::request_resolution(width, height, error);

            throw_if_error(error);

        }

        // void start_streaming()
        // {
        //     std::error_code error;
        //     Super::start_streaming(error);

        //     throw_if_error(error);
        // }

    };

    using camera =
        throw_if_error_layer<
        start_streaming_layer<
        set_h264_format_layer<

            request_format_layer<
        enumerate_formats_layer<
        read_capability_layer<
        open_layer<
            final_layer>>>>>>>;

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
