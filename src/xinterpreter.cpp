/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#include <algorithm>
#include <memory>
#include <regex>
#include <sstream>
#include <vector>

#include "xeus-cling/xbuffer.hpp"
#include "xeus-cling/xeus_cling_config.hpp"
#include "xeus-cling/xinterpreter.hpp"
#include "xeus-cling/xmagics.hpp"

#include "xinput.hpp"
#include "xinspect.hpp"
#include "xmagics/execution.hpp"
#include "xmagics/os.hpp"
#include "xmime_internal.hpp"
#include "xparser.hpp"
#include "xsystem.hpp"

using namespace std::placeholders;

namespace xcpp
{
    void interpreter::configure_impl()
    {
        // Process #include "xeus/xinterpreter.hpp" in a separate block.
        cling::Interpreter::CompilationResult compilation_result;
        m_processor.process("#include \"xeus/xinterpreter.hpp\"", compilation_result, nullptr, true);

        // Expose interpreter instance to cling
        std::string block = "xeus::register_interpreter(static_cast<xeus::xinterpreter*>((void*)" + std::to_string(intptr_t(this)) + "));";
        m_processor.process(block.c_str(), compilation_result, nullptr, true);
    }

    interpreter::interpreter(int argc, const char* const* argv)
        : m_cling(argc, argv, LLVM_DIR), m_processor(m_cling, cling::errs()),
          m_version(get_stdopt(argc, argv)), // Extract C++ language standard version from command-line option
          xmagics(),
          p_cout_strbuf(nullptr), p_cerr_strbuf(nullptr),
          m_cout_buffer(std::bind(&interpreter::publish_stdout, this, _1)),
          m_cerr_buffer(std::bind(&interpreter::publish_stderr, this, _1))
    {
        redirect_output();
        init_preamble();
        init_magic();
    }

    interpreter::~interpreter()
    {
        restore_output();
    }

    nl::json interpreter::execute_request_impl(int execution_counter,
                                               const std::string& code,
                                               bool silent,
                                               bool /*store_history*/,
                                               nl::json /*user_expressions*/,
                                               bool allow_stdin)
    {
        nl::json kernel_res;

        // Check for magics
        for (auto& pre : preamble_manager.preamble)
        {
            if (pre.second.is_match(code))
            {
                pre.second.apply(code, kernel_res);
                return kernel_res;
            }
        }

        // Split code from includes
        auto blocks = split_from_includes(code.c_str());

        auto errorlevel = 0;
        auto indent = 0;
        std::string ename;
        std::string evalue;
        cling::Value output;
        cling::Interpreter::CompilationResult compilation_result;

        // If silent is set to true, temporarily dismiss all std::cerr and
        // std::cout outpus resulting from `m_processor.process`.

        auto cout_strbuf = std::cout.rdbuf();
        auto cerr_strbuf = std::cerr.rdbuf();

        if (silent)
        {
            auto null = xnull();
            std::cout.rdbuf(&null);
            std::cerr.rdbuf(&null);
        }

        // Scope guard performing the temporary redirection of input requests.
        auto input_guard = input_redirection(allow_stdin);

        for (const auto& block : blocks)
        {
            // Attempt normal evaluation
            try
            {
                indent = m_processor.process(block, compilation_result, &output, true);
            }

            // Catch all errors
            catch (cling::InterpreterException& e)
            {
                errorlevel = 1;
                ename = "Interpreter Exception";
                if (!e.diagnose())
                {
                    evalue = e.what();
                }
            }
            catch (std::exception& e)
            {
                errorlevel = 1;
                ename = "Standard Exception";
                evalue = e.what();
            }
            catch (...)
            {
                errorlevel = 1;
                ename = "Error";
            }

            if (compilation_result != cling::Interpreter::kSuccess)
            {
                errorlevel = 1;
                ename = "Interpreter Error";
            }

            // If an error was encountered, don't attempt further execution
            if (errorlevel)
            {
                m_processor.cancelContinuation();
                break;
            }
        }

        // Flush streams
        std::cout << std::flush;
        std::cerr << std::flush;

        // Reset non-silent output buffers
        if (silent)
        {
            std::cout.rdbuf(cout_strbuf);
            std::cerr.rdbuf(cerr_strbuf);
        }

        // Depending of error level, publish execution result or execution
        // error, and compose execute_reply message.
        if (errorlevel)
        {
            // Classic Notebook does not make use of the "evalue" or "ename"
            // fields, and only displays the traceback.
            //
            // JupyterLab displays the "{ename}: {evalue}" if the traceback is
            // empty.
            std::vector<std::string> traceback({ename + ": " + evalue});
            if (!silent)
            {
                publish_execution_error(ename, evalue, traceback);
            }

            // Compose execute_reply message.
            kernel_res["status"] = "error";
            kernel_res["ename"] = ename;
            kernel_res["evalue"] = evalue;
            kernel_res["traceback"] = traceback;
        }
        else
        {
            // Publish a mime bundle for the last return value if
            // the semicolon was omitted.
            if (!silent && output.hasValue() && trim(blocks.back()).back() != ';')
            {
                nl::json pub_data = mime_repr(output);
                publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());
            }

            // Compose execute_reply message.
            kernel_res["status"] = "ok";
            kernel_res["payload"] = nl::json::array();
            kernel_res["user_expressions"] = nl::json::object();
        }
        return kernel_res;
    }

    nl::json interpreter::complete_request_impl(const std::string& code,
                                                int cursor_pos)
    {
        std::vector<std::string> result;
        cling::Interpreter::CompilationResult compilation_result;
        nl::json kernel_res;

        // split the input to have only the word in the back of the cursor
        std::string delims = " \t\n`!@#$^&*()=+[{]}\\|;:\'\",<>?.";
        std::size_t _cursor_pos = cursor_pos;
        auto text = split_line(code, delims, _cursor_pos);
        std::string to_complete = text.back().c_str();

        compilation_result = m_cling.codeComplete(code.c_str(), _cursor_pos, result);

        // change the print result
        for (auto& r : result)
        {
            // remove the definition at the beginning (for example [#int#])
            r = std::regex_replace(r, std::regex("\\[\\#.*\\#\\]"), "");
            // remove the variable name in <#type name#>
            r = std::regex_replace(r, std::regex("(\\ |\\*)+(\\w+)(\\#\\>)"), "$1$3");
            // remove unnecessary space at the end of <#type   #>
            r = std::regex_replace(r, std::regex("\\ *(\\#\\>)"), "$1");
            // remove <# #> to keep only the type
            r = std::regex_replace(r, std::regex("\\<\\#([^#>]*)\\#\\>"), "$1");
        }

        kernel_res["matches"] = result;
        kernel_res["cursor_start"] = cursor_pos - to_complete.length();
        kernel_res["cursor_end"] = cursor_pos;
        kernel_res["metadata"] = nl::json::object();
        kernel_res["status"] = "ok";
        return kernel_res;
    }

    nl::json interpreter::inspect_request_impl(const std::string& code,
                                               int cursor_pos,
                                               int /*detail_level*/)
    {
        nl::json kernel_res;

        auto dummy = code.substr(0, cursor_pos);
        // FIX: same pattern as in inspect function (keep only one)
        std::string exp = R"(\w*(?:\:{2}|\<.*\>|\(.*\)|\[.*\])?)";
        std::regex re_method{"(" + exp + R"(\.?)*$)"};
        std::smatch magic;
        if (std::regex_search(dummy, magic, re_method))
        {
            inspect(magic[0], kernel_res, m_processor);
        }
        return kernel_res;
    }

    nl::json interpreter::is_complete_request_impl(const std::string& /*code*/)
    {
        // TODO: use indentation returned from processing the code to determine
        // if the code is complete.
        nl::json kernel_res;
        kernel_res["status"] = "complete";
        kernel_res["indent"] = "";
        return kernel_res;
    }

    nl::json interpreter::kernel_info_request_impl()
    {
        nl::json result;
        result["implementation"] = "xeus-cling";
        result["implementation_version"] = XEUS_CLING_VERSION;

        /* The jupyter-console banner for xeus-cling is the following:
          __  _____ _   _ ___ 
          \ \/ / _ \ | | / __|
           >  <  __/ |_| \__ \
          /_/\_\___|\__,_|___/

          xeus-cling: a Jupyter Kernel C++ - based on cling
        */

        std::string banner = ""
              "  __  _____ _   _ ___\n"
              "  \\ \\/ / _ \\ | | / __|\n"
              "   >  <  __/ |_| \\__ \\\n"
              "  /_/\\_\\___|\\__,_|___/\n"
              "\n"
              "  xeus-cling: a Jupyter Kernel C++ - based on cling\n"
              "  C++";
        banner.append(m_version);
        result["banner"] = banner;
        result["language_info"]["name"] = "c++";
        result["language_info"]["version"] = m_version;
        result["language_info"]["mimetype"] = "text/x-c++src";
        result["language_info"]["codemirror_mode"] = "text/x-c++src";
        result["language_info"]["file_extension"] = ".cpp";
        result["status"] = "ok";
        return result;
    }

    void interpreter::shutdown_request_impl()
    {
        restore_output();
    }

    void interpreter::redirect_output()
    {
        p_cout_strbuf = std::cout.rdbuf();
        p_cerr_strbuf = std::cerr.rdbuf();

        std::cout.rdbuf(&m_cout_buffer);
        std::cerr.rdbuf(&m_cerr_buffer);
    }

    void interpreter::restore_output()
    {
        std::cout.rdbuf(p_cout_strbuf);
        std::cerr.rdbuf(p_cerr_strbuf);
    }

    void interpreter::publish_stdout(const std::string& s)
    {
        publish_stream("stdout", s);
    }

    void interpreter::publish_stderr(const std::string& s)
    {
        publish_stream("stderr", s);
    }

    void interpreter::init_preamble()
    {
        preamble_manager.register_preamble("introspection", new xintrospection(m_processor));
        preamble_manager.register_preamble("magics", new xmagics_manager());
        preamble_manager.register_preamble("shell", new xsystem());
    }

    void interpreter::init_magic()
    {
        preamble_manager["magics"].get_cast<xmagics_manager>().register_magic("file", writefile());
        preamble_manager["magics"].get_cast<xmagics_manager>().register_magic("timeit", timeit(&m_processor));
    }

    std::string interpreter::get_stdopt(int argc, const char* const* argv)
    {
        std::string res = "11";
        for (int i = 0; i < argc; ++i)
        {
            std::string tmp(argv[i]);
            auto pos = tmp.find("-std=c++");
            if (pos != std::string::npos)
            {
                res = tmp.substr(pos + 8);
                break;
            }
        }
        return res;
    }
}
