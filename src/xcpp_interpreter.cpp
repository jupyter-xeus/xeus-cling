/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <algorithm>
#include <regex>
#include <sstream>
#include <vector>
#include <memory>

#include "cling/Interpreter/Value.h"
#include "cling/Interpreter/Exception.h"
#include "cling/Utils/Output.h"

#include "xcpp_interpreter.hpp"
#include "xinspect.hpp"
#include "xparser.hpp"
#include "xbuffer.hpp"
#include "xmagics.hpp"

using namespace std::placeholders;

namespace xeus
{

    void xcpp_interpreter::configure_impl()
    {
        // Process #include "xeus/xinterpreter.hpp" in a separate block.
        cling::Value result;
        cling::Interpreter::CompilationResult compilation_result;
        m_processor.process("#include \"xeus/xinterpreter.hpp\"", compilation_result, &result);

        // Expose interpreter instance to cling
        std::string block = "xeus::register_interpreter(static_cast<xeus::xinterpreter*>((void*)" + std::to_string(intptr_t(this)) + "));";
        m_processor.process(block.c_str(), compilation_result, &result);
    }

    xcpp_interpreter::xcpp_interpreter(int argc, const char* const* argv)
        : m_cling(argc, argv, LLVM_DIR), m_processor(m_cling, cling::errs()),
          p_cout_strbuf(nullptr), p_cerr_strbuf(nullptr),
          m_cout_buffer(std::bind(&xcpp_interpreter::publish_stdout, this, _1)), 
          m_cerr_buffer(std::bind(&xcpp_interpreter::publish_stderr, this, _1)),
          xmagics()
    {
        redirect_output();
        init_magic();
    }

    xcpp_interpreter::~xcpp_interpreter()
    {
        restore_output();
    }

    xjson xcpp_interpreter::execute_request_impl(int execution_counter,
                                                 const std::string& code,
                                                 bool silent,
                                                 bool store_history,
                                                 const xjson_node* user_expressions,
                                                 bool allow_stdin)
    {
        cling::Value result;
        cling::Interpreter::CompilationResult compilation_result;
        xjson kernel_res;

        std::regex re_magic_cell("^\%{2}(\\w+)");
        std::smatch magic_name;
        if (std::regex_search(code, magic_name, re_magic_cell))
        {
            if (xmagics.find(magic_name.str(1)))
            {
                std::regex re_magic_cell("^\%{2}\\w+\\s(.*)\\n((?:.*\\n?)*)");
                std::smatch split_code;
                std::regex_search(code, split_code, re_magic_cell);
                xmagics.apply(magic_name[1], split_code[1], split_code[2]);
                xjson pub_data;
    
                // std::cout << m_cout_stream.str();
                // std::cerr << m_cerr_stream.str();

                // pub_data["text/plain"] = results.str() + errors.str();
                // publish_execution_result(execution_counter, std::move(pub_data), xjson::object());
                kernel_res["status"] = "ok";
            }
            else
            {
                // publish_execution_error("ename", "evalue", {"Unknown magic function"});
                kernel_res = get_error_reply("ename", "evalue", {});
            }
            return kernel_res;
        }

        auto blocks = split_from_includes(code.c_str());

        for (auto block : blocks)
        {
            std::regex re_magic_line("^\%(\\w+)(?:\\s(.*))?");
            std::smatch magic;
            if (std::regex_search(block, magic, re_magic_line))
            {
                if (xmagics.find(magic.str(1)))
                {
                    xmagics.apply(magic[1], magic[2]);
                }
                continue;
            }

            // Check for inspection requests
            std::regex re("\\?{2}");
            if (std::regex_search(block, re))
            {
                // Perform inspection.
                auto inspect_result = inspect(block, m_processor);

                // Format html content.
                std::string html_content = R"(<style>
                #pager-container {
                    padding: 0;
                    margin: 0;
                    width: 100%;
                    height: 100%;
                }
                .xeus-iframe-pager {
                    padding: 0;
                    margin: 0;
                    width: 100%;
                    height: 100%;
                    border: none;
                }
                </style>
                <iframe class="xeus-iframe-pager" src=")" + inspect_result + R"("></iframe>)";

                kernel_res["payload"] = { 
                    xjson::object({
                        {"data", {
                            {"text/plain", inspect_result}, 
                            {"text/html", html_content}}}, 
                        {"source", "page"}, 
                        {"start", 0}})
                };
                break;
            }
            else
            {
                // Perform normal evaluation
                auto errorlevel = 0;
                try
                {
                    errorlevel = m_processor.process(block.c_str(), compilation_result, &result);
                }
                catch (cling::InterpreterException& e)
                {
                    if (!e.diagnose())
                    {
                        std::cerr << "Caught an interpreter exception!\n"
                              << e.what() << '\n';
                    }
                }
                catch (std::exception& e)
                {
                    std::cerr << "Caught a std::exception!\n"
                              << e.what() << '\n';
                }
                catch (...)
                {
                    std::cerr << "Exception occurred. Recovering...\n";
                }

                if (errorlevel)
                {
                    m_processor.cancelContinuation();
                    kernel_res = get_error_reply("ename", "evalue", {});
                    return kernel_res;
                }
                else if (compilation_result != cling::Interpreter::kSuccess)
                {
                    kernel_res = get_error_reply("ename", "evalue", {});
                    return kernel_res;
                }
            }
        }

        std::cout << std::flush;
        kernel_res["status"] = "ok";
        return kernel_res;
    }

    xjson xcpp_interpreter::complete_request_impl(const std::string& code,
                                                  int cursor_pos)
    {
        std::vector<std::string> result;
        cling::Interpreter::CompilationResult compilation_result;
        xjson kernel_res;

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
        kernel_res["metadata"] = xjson::object();
        kernel_res["status"] = "ok";
        return kernel_res;
    }

    xjson xcpp_interpreter::inspect_request_impl(const std::string& code,
                                                 int cursor_pos,
                                                 int detail_level)
    {
        xjson kernel_res;
        xjson data;

        std::string code_copy = code;
        code_copy.replace(cursor_pos, 2, "??");
        auto result = inspect(code_copy, m_processor);

        if (result.empty())
        {
            kernel_res["found"] = false;
        }
        else
        {
            std::string html_content = "<pre><iframe style=\"width:100%; height:300px\" src=\"" + result + "\"></iframe></pre>";
            kernel_res["found"] = true;
            data["text/html"] = html_content;
            data["text/plain"] = result;
            kernel_res["data"] = data;
        }
        kernel_res["metadata"] = xjson::object();
        kernel_res["status"] = "ok";
        return kernel_res;
    }

    xjson xcpp_interpreter::history_request_impl(const xhistory_arguments& args)
    {
        return xjson::object();
    }

    xjson xcpp_interpreter::is_complete_request_impl(const std::string& code)
    {
        return xjson::object();
    }

    xjson xcpp_interpreter::kernel_info_request_impl()
    {
        xjson result;
        result["protocol_version"] = "5.0.0";
        result["implementation"] = "xeus-cling";
        result["implementation_version"] = "0.0.1";
        result["language_info"]["name"] = "c++";
        result["language_info"]["version"] = m_version;
        result["language_info"]["mimetype"] = "text/x-c++src";
        result["language_info"]["codemirror_mode"] = "text/x-c++src";
        result["language_info"]["file_extension"] = ".cpp";
        return result;
    }

    void xcpp_interpreter::input_reply_impl(const std::string& value)
    {
    }

    xjson xcpp_interpreter::get_error_reply(const std::string& ename,
                                            const std::string& evalue,
                                            const std::vector<std::string>& trace_back)
    {
        xjson result;
        result["status"] = "error";
        result["ename"] = ename;
        result["evalue"] = evalue;
        result["traceback"] = trace_back;
        return result;
    }

    void xcpp_interpreter::redirect_output()
    {
        p_cout_strbuf = std::cout.rdbuf();
        p_cerr_strbuf = std::cerr.rdbuf();

        std::cout.rdbuf(&m_cout_buffer);
        std::cerr.rdbuf(&m_cerr_buffer);
    }

    void xcpp_interpreter::restore_output()
    {
        std::cout.rdbuf(p_cout_strbuf);
        std::cerr.rdbuf(p_cerr_strbuf);
    }

    void xcpp_interpreter::publish_stdout(const std::string& s)
    {
        publish_stream("stdout", s);
    }

    void xcpp_interpreter::publish_stderr(const std::string& s)
    {
        publish_stream("stderr", s);

    void xcpp_interpreter::init_magic()
    {
        xmagics.register_magic("file", std::make_shared<writefile>());
        xmagics.register_magic("timeit", std::make_shared<timeit>(&m_processor));
    }
}
