// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include "capture_data.hpp"
#include "xu_h264_codec_control.hpp"
#include "trace_layer.hpp"

#include <memory>
#include <cstdint>

namespace cam
{

struct camera
{
    /// Constructor
    ///
    /// Note: Why we need to declare and define both constructor and destructor
    /// when using std::unique_ptr and pimpl:
    /// http://stackoverflow.com/a/9954553/1717320
    camera();

    /// Destructor (see constructor why definition of destructor is needed)
    ~camera();

    void try_open(const char* device);
    void open(const char* device, std::error_code& error);

    void try_request_resolution(uint32_t width, uint32_t height);
    void request_resolution(uint32_t width, uint32_t height,
                            std::error_code& error);

    uint32_t width() const;
    uint32_t height() const;
    std::string pixelformat() const;

    void try_start_streaming();
    void start_streaming(std::error_code& error);

    void try_stop_streaming();
    void stop_streaming(std::error_code& error);

    capture_data try_capture();
    capture_data capture(std::error_code& error);

    bool has_h264_codec_control() const;
    const xu_h264_codec_control& h264_codec_control() const;
    xu_h264_codec_control& h264_codec_control();

    void set_trace_stdout();
    void set_trace_off();
    void set_zone_prefix(const std::string& zone_prefix);
    const std::string& zone_prefix() const;
    void set_trace_callback(const trace_callback_function& callback);
    bool is_trace_enabled() const;

private:

    // Private implementation
    struct impl;
    std::unique_ptr<impl> m_impl;
};

}
