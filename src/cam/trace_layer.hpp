// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <meta/typelist.hpp>

#include <string>
#include <functional>
#include <iostream>

namespace cam
{
    /// @todo This is a copy/paste from kodo. If it is useful we should
    ///       make a sperate project for it
    using trace_callback_function =
        std::function<void(const std::string& zone, const std::string& data)>;

    /// @ingroup trace
    ///
    /// @brief When enabled the trace layer can be used to get human
    ///        readable information from a stack as it is running.
    ///
    /// Be aware that using the trace layers will most likely have a
    /// high impact on the performance of the stacks (i.e. making them
    /// significantly slower). Tracing should therefore only be used
    /// for debugging / understanding purposes.
    ///
    /// There are a bunch of different trace layers that will write
    /// information when tracing is enabled.
    ///
    /// As a user you can get this information by using the
    /// layer::set_trace_stdout() / layer::set_trace_callback(...) API.
    /// It is recommended that you use these functions defined in
    /// set_trace_stdout.hpp / set_trace_callback.hpp. This will allow your
    /// code to also work with stacks that do not have the tracing capabilities.
    ///
    /// If you look at the trace_callback_function function type you will see
    /// that it's a function which takes two arguments the zone and the data.
    /// The zone is a string which purpose is to allow filtering of the
    /// output. If you only want to see a specific part of the tracing
    /// output you can override the default trace_callback_function function
    /// and drop tracing data for zones which you are not interested
    /// in.
    ///
    /// If you are making your own layer and want to enable tracing
    /// it. This would be the general approach:
    ///
    ///     1. Lets assume your layer is called my_new_compressor then
    ///        you would create an additional layer called
    ///        trace_my_new_compressor.
    ///
    ///     2. The trace_my_new_compressor should intercept
    ///        "interesting" calls to the my_new_compressor layer and
    ///        write that information into a human readable string.
    ///
    ///     3. Write the tracing information using the
    ///        layer::write_trace(const std::string& zone, const
    ///        std::string& data) function exposed by this layer.
    ///
    ///     4. Try to avoid choosing a zone which is already in use by
    ///        other tracing layers. Since this will make it hard for
    ///        the user to filter.
    ///
    template<class SuperCoder>
    class trace_layer : public SuperCoder
    {
    public:

        /// @copydoc layer::initialize(Factory&)
        template<class Factory>
        void initialize(Factory& the_factory)
        {
            SuperCoder::initialize(the_factory);

            // Reset callback function to an empty callback function
            // and clear the zone prefix

            m_trace_callback = trace_callback_function();
            m_zone_prefix.clear();
        }

        /// @copydoc layer::set_trace_callback(const trace_callback_function&)
        void set_trace_callback(const trace_callback_function& callback)
        {
            assert(callback);
            m_trace_callback = callback;
        }

        /// @copydoc layer::set_trace_stdout()
        void set_trace_stdout()
        {
            m_trace_callback = [](const std::string& zone,
                                  const std::string& data)
                {
                    std::cout << zone << ": ";
                    std::cout << data << "\n";
                };
        }

        /// @copydoc layer::set_trace_off()
        void set_trace_off()
        {
            m_trace_callback = trace_callback_function();
        }

        /// @copydoc layer::trace_callback()
        const trace_callback_function& trace_callback() const
        {
            return m_trace_callback;
        }

        /// @copydoc layer::set_zone_prefix(const std::string&)
        void set_zone_prefix(const std::string& zone_prefix)
        {
            m_zone_prefix = zone_prefix;
        }

        /// @copydoc layer::zone_prefix()
        const std::string& zone_prefix() const
        {
            return m_zone_prefix;
        }

        /// @copydoc layer::write_trace(const std::string&, const std::string&)
        void write_trace(const std::string& zone,
                         const std::string& data) const
        {
            assert(m_trace_callback);

            if (m_zone_prefix.empty())
            {
                m_trace_callback(zone, data);
            }
            else
            {
                m_trace_callback(m_zone_prefix + "." + zone, data);
            }
        }

        /// @copydoc layer::is_trace_enabled()
        bool is_trace_enabled() const
        {
            return (bool) m_trace_callback;
        }

    private:

        /// The function where the tracing data will go
        trace_callback_function m_trace_callback;

        /// The zone prefix to append to the trace zone
        std::string m_zone_prefix;
    };
}
