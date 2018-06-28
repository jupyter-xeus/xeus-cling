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
    std::string file_name = (argc == 1) ? "connection.json" : extract_filename(argc, argv);
    xeus::xconfiguration config = xeus::load_configuration(file_name);

    interpreter_ptr interpreter = build_interpreter(argc, argv);
    xeus::xkernel kernel(config, xeus::get_user_name(), std::move(interpreter));
    std::cout << "starting kernel" << std::endl;
    kernel.start();

    return 0;
}
