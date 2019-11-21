// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include "camera.hpp"

#include <platform/config.hpp>

#ifdef PLATFORM_LINUX

#include "linux/camera.hpp"
using platform_camera = cam::linux::camera;

#else

#error "Platform not supported"

#endif

namespace cam
{

struct camera::impl : platform_camera
{ };

camera::camera() :
    m_impl(new camera::impl())
{ }

camera::~camera()
{ }

void camera::try_open(const std::string& device)
{
    assert(m_impl);
    m_impl->try_open(device);
}

void camera::open(const std::string& device, std::error_code& error)
{
    assert(m_impl);
    m_impl->open(device, error);
}

void camera::try_request_resolution(uint32_t width, uint32_t height)
{
    assert(m_impl);
    m_impl->try_request_resolution(width, height);
}

void camera::request_resolution(uint32_t width, uint32_t height,
                                std::error_code& error)
{
    assert(m_impl);
    m_impl->request_resolution(width, height, error);
}

uint32_t camera::width() const
{
    assert(m_impl);
    return m_impl->width();
}

uint32_t camera::height() const
{
    assert(m_impl);
    return m_impl->height();
}

std::string camera::pixelformat() const
{
    assert(m_impl);
    return m_impl->pixelformat();
}

void camera::try_start_streaming()
{
    assert(m_impl);
    m_impl->try_start_streaming();
}
void camera::start_streaming(std::error_code& error)
{
    assert(m_impl);
    m_impl->start_streaming(error);
}

void camera::try_stop_streaming()
{
    assert(m_impl);
    m_impl->try_stop_streaming();
}

void camera::stop_streaming(std::error_code& error)
{
    assert(m_impl);
    m_impl->stop_streaming(error);
}

capture_data camera::try_capture()
{
    assert(m_impl);
    return m_impl->try_capture();
}
capture_data camera::capture(std::error_code& error)
{
    assert(m_impl);
    return m_impl->capture(error);
}

bool camera::has_h264_codec_control() const
{
    assert(m_impl);
    return m_impl->has_h264_codec_control();
}
const xu_h264_codec_control& camera::h264_codec_control() const
{
    assert(m_impl);
    return m_impl->h264_codec_control();
}
xu_h264_codec_control& camera::h264_codec_control()
{
    assert(m_impl);
    return m_impl->h264_codec_control();
}

void camera::set_trace_stdout()
{
    assert(m_impl);
    m_impl->set_trace_stdout();
}

void camera::set_trace_off()
{
    assert(m_impl);
    m_impl->set_trace_off();
}

void camera::set_zone_prefix(const std::string& zone_prefix)
{
    assert(m_impl);
    m_impl->set_zone_prefix(zone_prefix);
}

const std::string& camera::zone_prefix() const
{
    assert(m_impl);
    return m_impl->zone_prefix();
}

void camera::set_trace_callback(const trace_callback_function& callback)
{
    assert(m_impl);
    m_impl->set_trace_callback(callback);
}

bool camera::is_trace_enabled() const
{
    assert(m_impl);
    return m_impl->is_trace_enabled();
}

}
