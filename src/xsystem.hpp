/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/
#ifndef XSYSTEM_HPP
#define XSYSTEM_HPP

#include <cstdio>

#include "xpreamble.hpp"

namespace xeus
{
    struct xsystem: xpreamble
    {
        const std::string spattern = R"(^\!)";

        xsystem()
        {
            xpreamble::set_pattern(spattern);
        }

        void apply(const std::string& code, xjson& kernel_res) override
        {
            std::regex re(spattern + R"((.*))");
            std::smatch to_execute;
            std::regex_search(code, to_execute, re);

            int ret=1;
    
            // redirection of stderr in stdout
            std::string command = to_execute.str(1) + " 2>&1";
            FILE *shell_result = popen(command.c_str(), "r");
            if (shell_result) {
                char buff[512];
                ret = 0;
                while (fgets(buff, sizeof(buff), shell_result))
                std::cout << buff;
                pclose(shell_result);

                std::cout << std::flush;
                kernel_res["status"] = "ok";
            }
            else
            {
                std::cerr << "Unable to execute the shell command\n";
                std::cout << std::flush;
                kernel_res["status"] = "error";
                kernel_res["ename"] = "ename";
                kernel_res["evalue"] = "evalue";
                kernel_res["traceback"] = {};            
            }
        }
    };
}
#endif