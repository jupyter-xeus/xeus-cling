/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XCPP_OPTIONS_HPP
#define XCPP_OPTIONS_HPP

#include <string>

#include <argparse/argparse.hpp>

namespace xcpp
{
    struct argparser : public argparse::ArgumentParser
    {
        using base_type = argparse::ArgumentParser;
        using base_type::ArgumentParser;

        void parse(const std::string& line);
    };
}
#endif
