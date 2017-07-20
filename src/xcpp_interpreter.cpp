/***************************************************************************
* Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <algorithm>
#include <regex>
#include <sstream>
#include <vector>

#include "xcpp_interpreter.hpp"
#include "xinspect.hpp"
#include "xparser.hpp"
#include "cling/Interpreter/Value.h"
#include "cling/Utils/Output.h"

namespace xeus
{
    xcpp_interpreter::xcpp_interpreter(int argc, const char* const* argv)
        : m_cling(argc, argv, LLVM_DIR), m_processor(m_cling, cling::errs()),
          p_cout_strbuf(nullptr), p_cerr_strbuf(nullptr), m_cout_stream(), m_cerr_stream()
    {
        redirect_output();
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

        auto blocks = split_from_includes(code.c_str());
        std::string res_value;

        for (auto block: blocks)
        {
            m_cout_stream.str("");
            m_cerr_stream.str("");
            if (m_processor.process(block.c_str(), compilation_result, &result))
            {
                m_processor.cancelContinuation();
                //TODO: pub_data.add_member("text/plain", "Incomplete input! Ignored.");
                publish_execution_error("ename", "evalue", {"Incomplete input"});
                kernel_res = get_error_reply("ename", "evalue", {});
                return kernel_res;
            }
            else if (compilation_result != cling::Interpreter::kSuccess)
            {
                std::string res_value = m_cerr_stream.str();
                publish_execution_error("ename", "evalue", { res_value });
                kernel_res = get_error_reply("ename", "evalue", {});
                return kernel_res;
            }
            else
                res_value += m_cout_stream.str();
        }

        if (!res_value.empty())
        {
            xjson pub_data{};
            pub_data["text/plain"] =res_value;
            publish_execution_result(execution_counter, std::move(pub_data), xjson());
        }
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
        std::string delims = " \t\n`!@#$^&*()=+[{]}\\|;:\'\",<>?";
        std::size_t _cursor_pos = cursor_pos;
        auto text = split_line(code, delims, _cursor_pos);
        std::string to_complete = text.back().c_str();

        compilation_result = m_cling.codeComplete(code.c_str(), _cursor_pos, result);

        // change the print result
        for(auto& r: result)
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
        kernel_res["metadata"] = xjson();
        kernel_res["status"] = "ok";
        return kernel_res;
    }

    xjson xcpp_interpreter::inspect_request_impl(const std::string& code,
                                                 int cursor_pos,
                                                 int detail_level)
    {
        xjson kernel_res;
        xjson data(kernel_res.get_allocator());
     
        auto result = inspect(code, cursor_pos);
        
        if (result.empty())
            kernel_res.set_value("/found", false);
        else
        {
            std::string html_content = "<pre><iframe style=\"width:100%; height:300px\" src=\"" + result + "\"></iframe></pre>"; 
        // std::string text_content = "http://en.cppreference.com/w/cpp/container/vector"; 
            kernel_res.set_value("/found", true);
            data.add_member("text/html", html_content);
            data.add_member("text/plain", result);
            kernel_res.add_subtree("data", data);
        }
        kernel_res.add_subtree("metadata", xjson{});
        kernel_res.set_value("/status", "ok");
        return kernel_res;
    }

    xjson xcpp_interpreter::history_request_impl(const xhistory_arguments& args)
    {
        return xjson();
    }

    xjson xcpp_interpreter::is_complete_request_impl(const std::string& code)
    {
        return xjson();
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

        std::cout.rdbuf(m_cout_stream.rdbuf());
        std::cerr.rdbuf(m_cerr_stream.rdbuf());
    }

    void xcpp_interpreter::restore_output()
    {
        std::cout.rdbuf(p_cout_strbuf);
        std::cerr.rdbuf(p_cerr_strbuf);
    }

}
