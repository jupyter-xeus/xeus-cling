/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/
#include <sstream>
#include <string>
#include <vector>

#include "xoptions.hpp"

namespace xeus
{
    void xoptions::parse(const std::string& line)
    {
        std::istringstream iss(line);
        std::vector<std::string> opt_strings((std::istream_iterator<std::string>(iss)),
                                             std::istream_iterator<std::string>());

        std::vector<char*> copt_strings;

        for (std::size_t i = 0; i < opt_strings.size(); ++i)
            copt_strings.push_back(const_cast<char*>(opt_strings[i].c_str()));

        int argc = copt_strings.size();
        auto argv = &copt_strings[0];
        parent::parse(argc, argv);
    }
}
