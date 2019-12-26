/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XMAGICS_OS_HPP
#define XMAGICS_OS_HPP

#include <string>

#include "xeus-cling/xmagics.hpp"
#include "xeus-cling/xoptions.hpp"

namespace xcpp
{
    class writefile: public xmagic_cell
    {
    public:

        xoptions get_options();
        virtual void operator()(const std::string& line, const std::string& cell) override;

    private:

        static bool is_file_exist(const char* fileName);
    };
}
#endif
