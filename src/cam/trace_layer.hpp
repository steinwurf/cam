// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

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


    using default_features = meta::typelist<>;


    /// @copydoc layer::trace_callback_function
    ///
    /// Type trait helper allows compile time detection of whether a
    /// codec contains a layer with the member function
    /// layer::set_trace_callback(const trace_callback_function&)
    ///
    /// Example:
    ///
    /// using encoder_t kodo::full_rlnc8_encoder;
    ///
    /// if (kodo::has_set_trace_callback<encoder_t>::value)
    /// {
    ///     // Do something here
    /// }
    ///
    template<typename T>
    struct has_set_trace_callback
    {
    private:
        using yes = std::true_type;
        using no = std::false_type;

        // Here we check that the layer has the
        // layer::set_trace_callback(const trace_callback_function&) function.
        template<typename U>
        static auto test(int) ->
            decltype(std::declval<U>().set_trace_callback(
                         std::declval<trace_callback_function>()),
                     yes());

        template<typename> static no test(...);

    public:

        static const bool value =
            std::is_same<decltype(test<T>(0)),yes>::value;
    };

    /// @ingroup type_traits trace
    ///
    /// Type trait helper allows compile time detection of whether a codec
    /// contains a layer with the member function layer::set_trace_stdout()
    ///
    /// Example:
    ///
    /// using encoder_t kodo::full_rlnc8_encoder;
    ///
    /// if (kodo::has_set_trace_stdout<encoder_t>::value)
    /// {
    ///     // Do something here
    /// }
    ///
    template<typename T>
    struct has_set_trace_stdout
    {
    private:
        using yes = std::true_type;
        using no = std::false_type;

        // Here we check that the layer has the layer::set_trace_stdout()
        // function.
        template<typename U>
        static auto test(int) ->
            decltype(std::declval<U>().set_trace_stdout(), yes());

        template<typename> static no test(...);

    public:

        static const bool value =
            std::is_same<decltype(test<T>(0)),yes>::value;
    };

    /// @ingroup type_traits trace
    ///
    /// Type trait helper allows compile time detection of whether a codec
    /// contains a layer with the member function layer::set_trace_off()
    ///
    /// Example:
    ///
    /// using encoder_t kodo::full_rlnc8_encoder;
    ///
    /// if (kodo::has_set_trace_off<encoder_t>::value)
    /// {
    ///     // Do something here
    /// }
    ///
    template<typename T>
    struct has_set_trace_off
    {
    private:
        using yes = std::true_type;
        using no = std::false_type;

        // Here we check that the layer has the layer::set_trace_off()
        // function.
        template<typename U>
        static auto test(int) ->
            decltype(std::declval<U>().set_trace_off(), yes());

        template<typename> static no test(...);

    public:

        static const bool value =
            std::is_same<decltype(test<T>(0)),yes>::value;
    };

    /// @ingroup generic_api trace
    ///
    /// This function calls the layer::set_trace_callback(Callback)
    /// function used to produce a trace for the chosen codec.
    template
    <
        class T,
        class Callback,
        typename std::enable_if<
            has_set_trace_callback<T>::value, uint8_t>::type = 0
    >
    static void set_trace_callback(T& t, const Callback& callback)
    {
        t.set_trace_callback(callback);
    }

    /// @ingroup generic_api trace
    ///
    /// @copydoc set_trace_callback(const T&, const Callback&)
    template
    <
        class T,
        class Callback,
        typename std::enable_if<
            !has_set_trace_callback<T>::value, uint8_t>::type = 0
    >
    static void set_trace_callback(T& t, const Callback& callback)
    {
        (void) t;
        (void) callback;

        // We do the assert here - to make sure that this call is not
        // silently ignored in cases where the stack does not have the
        // set_trace_callback() function. However, this assert can
        // be avoided by using the has_set_trace_callback.
        assert(0);
    }

    /// Tag used to to enable tracing
    struct enable_trace
    { };

    /// @ingroup trace
    ///
    /// @brief Very simple type trait to check whether a type is enable_trace
    ///
    template<class Feature>
    using is_enable_trace = std::is_same<Feature, enable_trace>;

    /// @todo This is a copy/paste from kodo. If it is useful we should
    ///       make a sperate project for it
    template<class Features>
    using find_enable_trace =
        typename Features::template find<is_enable_trace, meta::not_found>;

    /// Fall-through case for the case where TraceTag is meta::not_found
    template<class TraceTag, class SuperCoder>
    class trace_layer : public SuperCoder
    {
        static_assert(std::is_same<TraceTag, meta::not_found>::value,
                      "Unexpected TraceTag should be meta::not_found in the "
                      "fall-through case.");
    };

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
    class trace_layer<enable_trace, SuperCoder> : public SuperCoder
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
                    std::cout << zone << ":" << std::endl;
                    std::cout << data << std::endl;
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
