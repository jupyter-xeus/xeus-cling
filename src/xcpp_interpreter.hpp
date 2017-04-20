/***************************************************************************
* Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XCPP_INTERPRETER_HPP
#define XCPP_INTERPRETER_HPP

#include "cling/Interpreter/Interpreter.h"
#include "cling/MetaProcessor/MetaProcessor.h"
#include "xeus/xinterpreter.hpp"

namespace xeus
{

    class xcpp_interpreter : public xinterpreter
    {

    public:

        xcpp_interpreter(int argc, const char* const* argv);
        virtual ~xcpp_interpreter() = default;

    private:

        xjson execute_request_impl(int execution_counter,
                                   const std::string& code,
                                   bool silent,
                                   bool store_history,
                                   const xjson::node_type* user_expressions,
                                   bool allow_stdin) override;

        xjson complete_request_impl(const std::string& code,
                                    int cursor_pos) override;

        xjson inspect_request_impl(const std::string& code,
                                   int cursor_pos,
                                   int detail_level) override;

        xjson history_request_impl(const xhistory_arguments& args) override;

        xjson is_complete_request_impl(const std::string& code) override;

        xjson comm_info_request_impl(const std::string& target_name) override;

        xjson kernel_info_request_impl() override;

        void input_reply_impl(const std::string& value) override;

        xjson get_error_reply(const std::string& ename,
                              const std::string& evalue,
                              const std::vector<std::string>& trace_back);

        cling::Interpreter m_cling;
        cling::MetaProcessor m_processor;
        std::string m_version;
    };
}

#endif

