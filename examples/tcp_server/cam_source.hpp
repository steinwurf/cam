// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <functional>
#include <cstdint>
#include <string>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <utility>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>


#include <sak/convert_endian.hpp>
#include <nalu/to_annex_b_nalus.hpp>

#include <cam/linux/linux.hpp>
#include <cam/linux/camera.hpp>

#include <cam/split_capture_on_nalu_type.hpp>

#include "get_option.hpp"

class cam_source
{
public:

    using handle_data_callback =
        std::function<void(const std::vector<uint8_t>&, uint64_t)>;

public:

    cam_source(
        boost::asio::io_service* io_service,
        const std::string& camera_device,
        const boost::program_options::variables_map& vm):
        m_io_service(io_service),
        m_camera_device(camera_device),
        m_variables_map(vm)
    { }

    void init()
    {
        m_frames = 0;
        m_diff_timestamp = 0;
        m_previous_timestamp = 0;

        m_camera = std::make_shared<cam::linux::camera<cam::default_features>>();
        m_camera->set_io_service(m_io_service);

        m_camera->try_open(m_camera_device.c_str());

        m_camera->try_request_resolution(400, 500);

        if (m_variables_map.count("i_frame_period"))
        {
            m_camera->try_request_i_frame_period(
                get_option<uint32_t>(m_variables_map, "i_frame_period"));
        }

        m_camera->try_start_streaming();

        if (m_variables_map.count("bitrate"))
        {
            auto bitrate = get_option<uint32_t>(m_variables_map, "bitrate");
            m_camera->try_request_bitrates(bitrate, bitrate);
        }
    }

    void start(handle_data_callback callback)
    {
        m_callback = callback;
        m_camera->async_capture(std::bind(
            &cam_source::do_stream, this, std::placeholders::_1));
    }

    void stop()
    {
        m_camera->try_stop_streaming();
    }

private:

    void do_stream(boost::system::error_code error)
    {
        if (error)
        {
            /// @todo Fix so that it works later, now just crash and burn.
            assert(0);
            return;
        }
        do_read();
        m_camera->async_capture(std::bind(
            &cam_source::do_stream, this, std::placeholders::_1));
    }

    void do_read()
    {
        assert(m_camera);
        auto data = m_camera->try_capture();
        assert(data);

        // check if the timestamp is valid.
        // The timestamp is expected to be higher than the last.
        // The Logitech 920c camera can some times output timeframes which
        // are 400 times higher than the previous. This have been observed
        // when keeping ones hand above the camera lense for ~30 seconds.
        // This is obviously bogus, and hence we check that the timestamp
        // is no more than 10 times larger than the last.
        if (data.m_timestamp < m_previous_timestamp)
        {
            return;
        }

        if (m_previous_timestamp != 0 &&
            data.m_timestamp / m_previous_timestamp > 10)
        {
            return;
        }

        assert(data.m_timestamp >= m_previous_timestamp);

        m_diff_timestamp = data.m_timestamp - m_previous_timestamp;
        m_previous_timestamp = data.m_timestamp;

        auto split_captures = cam::split_capture_on_nalu_type(data);
        for (const auto& c : split_captures)
        {
            std::cout << m_frames << ": " << c << " diff_timestamp = "
                      << m_diff_timestamp << std::endl;

            auto nalus = nalu::to_annex_b_nalus(c.m_data, c.m_size);

            for (const auto& nalu : nalus)
            {
                std::cout << "  " << nalu << std::endl;
                assert(nalu);
            }

            std::vector<uint8_t> v(c.m_data, c.m_data + c.m_size);
            m_callback(v, c.m_timestamp);

            ++m_frames;
        }
    }

private:

    boost::asio::io_service* m_io_service;
    std::string m_camera_device;
    boost::program_options::variables_map m_variables_map;

    std::shared_ptr<cam::linux::camera<cam::default_features>> m_camera;

    // Counts the number of NALUs
    uint32_t m_frames = 0;
    uint32_t m_diff_timestamp = 0;
    uint32_t m_previous_timestamp = 0;

    handle_data_callback m_callback;
};
