/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "xeus/xkernel.hpp"
#include "xeus/xkernel_configuration.hpp"

#include "xeus-cling/xinterpreter.hpp"

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
    // If we are called from the Jupyter launcher, silence all logging. This
    // is important for a JupyterHub configured with cleanup_servers = False:
    // Upon restart, spawned single-user servers keep running but without the
    // std* streams. When a user then tries to start a new kernel, xeus-cling
    // will get a SIGPIPE when writing to any of these and exit.
    if (std::getenv("JPY_PARENT_PID") != NULL)
    {
        std::clog.setstate(std::ios_base::failbit);
    }

    std::string file_name = extract_filename(argc, argv);

    interpreter_ptr interpreter = build_interpreter(argc, argv);

    if (!file_name.empty())
    {
        xeus::xconfiguration config = xeus::load_configuration(file_name);

        xeus::xkernel kernel(config, xeus::get_user_name(), std::move(interpreter));

        std::clog <<
            "Starting xeus-cling kernel...\n\n"
            "If you want to connect to this kernel from an other client, you can use"
            " the " + file_name + " file."
            << std::endl;

        kernel.start();
    }
    else
    {
        xeus::xkernel kernel(xeus::get_user_name(), std::move(interpreter));

        const auto& config = kernel.get_config();
        std::clog <<
            "Starting xeus-cling kernel...\n\n"
            "If you want to connect to this kernel from an other client, just copy"
            " and paste the following content inside of a `kernel.json` file. And then run for example:\n\n"
            "# jupyter console --existing kernel.json\n\n"
            "kernel.json\n```\n{\n"
            "    \"transport\": \"" + config.m_transport + "\",\n"
            "    \"ip\": \"" + config.m_ip + "\",\n"
            "    \"control_port\": " + config.m_control_port + ",\n"
            "    \"shell_port\": " + config.m_shell_port + ",\n"
            "    \"stdin_port\": " + config.m_stdin_port + ",\n"
            "    \"iopub_port\": " + config.m_iopub_port + ",\n"
            "    \"hb_port\": " + config.m_hb_port + ",\n"
            "    \"signature_scheme\": \"" + config.m_signature_scheme + "\",\n"
            "    \"key\": \"" + config.m_key + "\"\n"
            "}\n```\n";

        kernel.start();
    }

    return 0;
}
