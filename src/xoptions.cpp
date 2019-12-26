/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#include <sstream>
#include <string>
#include <vector>

#include "xeus-cling/xoptions.hpp"

namespace xcpp
{
    cxxopts::ParseResult xoptions::parse(const std::string& line)
    {
        std::istringstream iss(line);
        std::vector<std::string> opt_strings((std::istream_iterator<std::string>(iss)),
                                              std::istream_iterator<std::string>());

        std::vector<const char*> copt_strings;

        for (std::size_t i = 0; i < opt_strings.size(); ++i)
        {
            copt_strings.push_back(opt_strings[i].c_str());
        }

        int argc = copt_strings.size();

        // Const-casting as cxxopts::parse moved from (int&, const char**&) to
        // (int&, char**&) between 2.1.0 and 2.1.1.).
        // This should not be required in 2.2.0.
        //
        // Macros CXXOPTS__VERSION_[MAJOR/MINOR/PATCH] were only defined
        // after 2.1.1.]
        auto argv = const_cast<char**>(&copt_strings[0]);
        return parent::parse(argc, argv);
    }
}
