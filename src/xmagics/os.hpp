/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/
#include <string>

#include "../xmagics.hpp"

namespace xeus
{
    struct writefile: public xmagic_cell
    {
        virtual void operator()(const std::string& line, const std::string& cell) const override;
        static bool is_file_exist(const char* fileName);
    };
}