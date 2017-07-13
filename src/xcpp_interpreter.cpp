/***************************************************************************
* Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "xcpp_interpreter.hpp"
#include "cling/Interpreter/Value.h"
#include "cling/Utils/Output.h"

namespace xeus
{

    xcpp_interpreter::xcpp_interpreter(int argc, const char* const* argv)
        : m_cling(argc, argv, LLVM_DIR), m_processor(m_cling, cling::errs())
    {
    }

    std::string value_to_string(const cling::Value& value)
    {
        std::string value_string;
        llvm::raw_string_ostream os(value_string);
        value.print(os);
        return value_string;
    }

    xjson xcpp_interpreter::execute_request_impl(int execution_counter,
                                                 const std::string& code,
                                                 bool silent,
                                                 bool store_history,
                                                 const xjson::node_type* user_expressions,
                                                 bool allow_stdin)
    {
        cling::Value result;
        cling::Interpreter::CompilationResult compilation_result;
        xjson kernel_res;

        if (m_processor.process(code.c_str(), compilation_result, &result))
        {
            m_processor.cancelContinuation();
            //TODO: pub_data.add_member("text/plain", "Incomplete input! Ignored.");
            publish_execution_error("ename", "evalue", {});
            kernel_res = get_error_reply("ename", "evalue", {});
        }
        else if (compilation_result != cling::Interpreter::kSuccess)
        {
            publish_execution_error("ename", "evalue", {});
            kernel_res = get_error_reply("ename", "evalue", {});
        }
        else if (!result.isValid())
        {
            publish_execution_error("ename", "evalue", {});
            kernel_res = get_error_reply("ename", "evalue", {});
        }
        else
        {
            xjson pub_data;
            std::string res_value = value_to_string(result);
            pub_data.add_member("text/plain", res_value);
            kernel_res.set_value("/status", "ok");
            publish_execution_result(execution_counter, std::move(pub_data), xjson());
        }

        return kernel_res;
    }

    xjson xcpp_interpreter::complete_request_impl(const std::string& code,
                                                  int cursor_pos)
    {
        return xjson();
    }

    xjson xcpp_interpreter::inspect_request_impl(const std::string& code,
                                                 int cursor_pos,
                                                 int detail_level)
    {
        return xjson();
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
        result.set_value("/protocol_version", "5.0.0");
        result.set_value("/implementation", "xeus-cling");
        result.set_value("/implementation_version", "0.0.1");
        result.set_value("/language_info/name", "c++");
        result.set_value("/language_info/version", m_version);
        result.set_value("/language_info/mimetype", "text/x-c++src");
        result.set_value("/codemirror_mode", "text/x-c++src");
        result.set_value("/language_info/file_extension", ".cpp");
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
        result.set_value("/status", "error");
        result.set_value("/ename", ename);
        result.set_value("/evalue", evalue);
        result.set_value("/traceback", trace_back);
        return result;
    }
}
