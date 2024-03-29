// Copyright (c) 2016 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <string>
#include <boost/program_options.hpp>

/// Small wrapper to check if an option exists before getting it. If it
/// does not exist it prints. Otherwise it is hard to know what failed.
template<class T>
inline T get_option(const boost::program_options::variables_map& vm,
                    const std::string& option)
{
    T type;

    try
    {
        type = vm[option].as<T>();
    }
    catch (const std::exception& e)
    {
        std::cout << "Trying to get option " << option << " failed with: "
                  << e.what() << "\n" << std::endl;

        throw e;
    }

    return type;
}
