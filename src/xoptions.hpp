/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XCPP_OPTIONS_HPP
#define XCPP_OPTIONS_HPP

#include "cxxopts.hpp"

namespace xeus
{
    struct xoptions : public cxxopts::Options
    {
        using parent = cxxopts::Options;
        using parent::Options;
        void parse(const std::string& line);
    };
}
#endif
