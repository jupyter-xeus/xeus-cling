/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "xinterpreter.hpp"
#include "xeus/xkernel.hpp"
#include "xeus/xkernel_configuration.hpp"
#include <iostream>
#include <memory>
#include <string>

std::string extract_filename(int& argc, char* argv[])
{
    std::string res = "";
    for (int i = 0; i < argc; ++i)
    {
        if ((std::string(argv[i]) == "-f") && (i + 1 < argc))
        {
            res = argv[i + 1];
            for(int j = i; j < argc - 2; ++j)
            {
                argv[j]  = argv[j + 2];
            }
            argc -= 2;
            break;
        }
    }
    return res;
}


using interpreter_ptr = std::unique_ptr<xcpp::interpreter>;
interpreter_ptr build_interpreter(int argc, char** argv)
{
    int interpreter_argc = argc + 1;
    const char** interpreter_argv = new const char*[interpreter_argc];
    interpreter_argv[0] = "xeus-cling";
    // Copy all arguments in the new array excepting the process name.
    for(int i = 1; i < argc; i++)
    {
        interpreter_argv[i] = argv[i];
    }
    std::string include_dir = std::string(LLVM_DIR) + std::string("/include");
    interpreter_argv[interpreter_argc - 1] = include_dir.c_str();

    interpreter_ptr interp_ptr = interpreter_ptr(new xcpp::interpreter(interpreter_argc, interpreter_argv));
    delete[] interpreter_argv;
    return interp_ptr;
}

int main(int argc, char* argv[])
{
    std::string file_name = extract_filename(argc, argv);
    if (file_name.compare("") == 0)
    {
        std::cout <<
              "\033[31;1;4mError\033[0m: The xeus-cling kernel needs a connection file.\n\n"
              "If you are trying to run xeus-cling in the console, please run one of the following commands "
              "(Please make sure that `jupyter` and `jupyter_console` packages are installed first):\n"
              "```\n"
              "# For a C++ 11 interpreter:\n"
              "jupyter console --kernel=xeus-cling-cpp11\n\n"
              "# C++ 14:\n"
              "jupyter console --kernel=xeus-cling-cpp14\n\n"
              "# C++ 17:\n"
              "jupyter console --kernel=xeus-cling-cpp17\n"
              "```" << std::endl;
        return 1;
    }

    xeus::xconfiguration config = xeus::load_configuration(file_name);

    interpreter_ptr interpreter = build_interpreter(argc, argv);
    xeus::xkernel kernel(config, xeus::get_user_name(), std::move(interpreter));
    std::cout << "starting kernel" << std::endl;
    kernel.start();

    return 0;
}
