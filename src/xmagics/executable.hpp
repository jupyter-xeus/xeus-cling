/***********************************************************************************
* Copyright (c) 2020, Jonas Hahnfeld                                               *
* Copyright (c) 2020, Chair for Computer Science 12 (HPC), RWTH Aachen University  *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XMAGICS_EXECUTABLE_HPP
#define XMAGICS_EXECUTABLE_HPP

#include <string>
#include <vector>

#include "cling/Interpreter/Interpreter.h"

#include "xeus-cling/xmagics.hpp"
#include "xeus-cling/xoptions.hpp"

namespace xcpp
{
    class executable: public xmagic_cell
    {
    public:

        executable(cling::Interpreter& i) : m_interpreter(i) {}
        xoptions get_options();
        virtual void operator()(const std::string& line, const std::string& cell) override;

    private:

        std::string generate_fns(const std::string& cell, std::string& main,
                                 std::string& unique_fn);
        bool generate_obj(std::string& ObjectFile, bool EnableDebugInfo);
        bool generate_exe(const std::string& ObjectFile,
                          const std::string& ExeFile,
                          const std::vector<std::string>& LinkerOptions);

        cling::Interpreter& m_interpreter;
        unsigned int m_unique = 0;
    };
}
#endif
