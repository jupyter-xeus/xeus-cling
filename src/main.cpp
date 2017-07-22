/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "xcpp_interpreter.hpp"
#include "xeus/xkernel.hpp"
#include "xeus/xkernel_configuration.hpp"
#include <iostream>
#include <memory>
#include <string>

std::string get_filename(int argc, char* argv[])
{
    std::string res = "";
    for (int i = 0; i < argc; ++i)
    {
        if ((std::string(argv[i]) == "-f") && (i + 1 < argc))
        {
            res = argv[i + 1];
            break;
        }
    }
    return res;
}

std::string get_stdopt(int argc, char** argv)
{
    std::string res = "-std=c++11";
    for (int i = 0; i < argc; ++i)
    {
        std::string tmp(argv[i]);
        if (tmp.find("-std=c++") != std::string::npos)
        {
            res = tmp;
            break;
        }
    }
    return res;
}

using interpreter_ptr = std::unique_ptr<xeus::xcpp_interpreter>;
interpreter_ptr build_interpreter(int argc, char** argv)
{
    constexpr int interpreter_argc = 5;
    const char* interpreter_argv[interpreter_argc];
    interpreter_argv[0] = "clingJupyter";
    std::string stdopt = get_stdopt(argc, argv);
    interpreter_argv[1] = stdopt.c_str();
    std::string include_dir = std::string(LLVM_DIR) + std::string("/include");
    interpreter_argv[2] = include_dir.c_str();
    interpreter_argv[3] = "";
    interpreter_argv[4] = "";
    return interpreter_ptr(new xeus::xcpp_interpreter(interpreter_argc, interpreter_argv));
}

int main(int argc, char* argv[])
{
    std::string file_name = (argc == 1) ? "connection.json" : get_filename(argc, argv);
    xeus::xconfiguration config = xeus::load_configuration(file_name);

    interpreter_ptr interpreter = build_interpreter(argc, argv);
    xeus::xkernel kernel(config, xeus::get_user_name(), std::move(interpreter));
    std::cout << "starting kernel" << std::endl;
    kernel.start();

    return 0;
}
