/************************************************************************************
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

#include <signal.h>

#ifdef __GNUC__
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#include <xeus/xkernel.hpp>
#include <xeus/xkernel_configuration.hpp>

#include <xeus-zmq/xserver_zmq.hpp>

#include "xeus-cling/xeus_cling_config.hpp"
#include "xeus-cling/xinterpreter.hpp"

#ifdef __GNUC__
void handler(int sig)
{
    void* array[10];

    // get void*'s for all entries on the stack
    size_t size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
#endif

void stop_handler(int /*sig*/)
{
    exit(0);
}

bool should_print_version(int argc, char* argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--version")
        {
            return true;
        }
    }
    return false;
}

std::string extract_filename(int *argc, char* argv[])
{
    std::string res = "";
    for (int i = 0; i < *argc; ++i)
    {
        if ((std::string(argv[i]) == "-f") && (i + 1 < *argc))
        {
            res = argv[i + 1];
            for (int j = i; j < *argc - 2; ++j)
            {
                argv[j] = argv[j + 2];
            }
            *argc -= 2;
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
    for (int i = 1; i < argc; i++)
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
    if (should_print_version(argc, argv))
    {
        std::clog << "xcpp " << XEUS_CLING_VERSION << std::endl;
        return 0;
    }

    // If we are called from the Jupyter launcher, silence all logging. This
    // is important for a JupyterHub configured with cleanup_servers = False:
    // Upon restart, spawned single-user servers keep running but without the
    // std* streams. When a user then tries to start a new kernel, xeus-cling
    // will get a SIGPIPE when writing to any of these and exit.
    if (std::getenv("JPY_PARENT_PID") != NULL)
    {
        std::clog.setstate(std::ios_base::failbit);
    }

    // Registering SIGSEGV handler
#ifdef __GNUC__
    std::clog << "registering handler for SIGSEGV" << std::endl;
    signal(SIGSEGV, handler);

    // Registering SIGINT and SIGKILL handlers
    signal(SIGKILL, stop_handler);
#endif
    signal(SIGINT, stop_handler);

    std::string file_name = extract_filename(&argc, argv);

    interpreter_ptr interpreter = build_interpreter(argc, argv);

    auto context = xeus::make_context<zmq::context_t>();

    if (!file_name.empty())
    {
        xeus::xconfiguration config = xeus::load_configuration(file_name);

        xeus::xkernel kernel(
            config,
            xeus::get_user_name(),
            std::move(context),
            std::move(interpreter),
            xeus::make_xserver_zmq,
            xeus::make_in_memory_history_manager(),
            xeus::make_console_logger(
                xeus::xlogger::msg_type,
                xeus::make_file_logger(xeus::xlogger::content, "xeus.log")
            )
        );

        std::clog << "Starting xeus-cling kernel...\n\n"
                     "If you want to connect to this kernel from an other client, you can use"
                     " the "
                         + file_name + " file."
                  << std::endl;

        kernel.start();
    }
    else
    {
        xeus::xkernel kernel(
            xeus::get_user_name(),
            std::move(context),
            std::move(interpreter),
            xeus::make_xserver_zmq,
            xeus::make_in_memory_history_manager(),
            xeus::make_console_logger(
                xeus::xlogger::msg_type,
                xeus::make_file_logger(xeus::xlogger::content, "xeus.log")
            )
        );

        const auto& config = kernel.get_config();
        std::clog << "Starting xeus-cling kernel...\n\n"
                     "If you want to connect to this kernel from an other client, just copy"
                     " and paste the following content inside of a `kernel.json` file. And then run for example:\n\n"
                     "# jupyter console --existing kernel.json\n\n"
                     "kernel.json\n```\n{\n"
                     "    \"transport\": \""
                         + config.m_transport
                         + "\",\n"
                           "    \"ip\": \""
                         + config.m_ip
                         + "\",\n"
                           "    \"control_port\": "
                         + config.m_control_port
                         + ",\n"
                           "    \"shell_port\": "
                         + config.m_shell_port
                         + ",\n"
                           "    \"stdin_port\": "
                         + config.m_stdin_port
                         + ",\n"
                           "    \"iopub_port\": "
                         + config.m_iopub_port
                         + ",\n"
                           "    \"hb_port\": "
                         + config.m_hb_port
                         + ",\n"
                           "    \"signature_scheme\": \""
                         + config.m_signature_scheme
                         + "\",\n"
                           "    \"key\": \""
                         + config.m_key
                         + "\"\n"
                           "}\n```\n";

        kernel.start();
    }

    return 0;
}
