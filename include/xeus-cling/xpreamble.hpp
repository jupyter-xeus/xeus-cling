/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XCPP_PREAMBLE_HPP
#define XCPP_PREAMBLE_HPP

#include <regex>
#include <string>

#include "nlohmann/json.hpp"

namespace nl = nlohmann;

namespace xcpp
{
    struct xpreamble
    {
        std::regex pattern;

        bool is_match(const std::string& s) const
        {
            std::smatch match;
            return std::regex_search(s, match, pattern);
        }

        virtual void apply(const std::string& s, nl::json& kernel_res) = 0;
        virtual xpreamble* clone() const = 0;
        virtual ~xpreamble() {};
    };
}
#endif
