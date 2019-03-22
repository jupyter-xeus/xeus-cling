/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_CLING_INTERPRETER_HPP
#define XEUS_CLING_INTERPRETER_HPP

#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#include "cling/Interpreter/Interpreter.h"
#include "cling/MetaProcessor/MetaProcessor.h"

#include "xeus/xinterpreter.hpp"

#include "xeus_cling_config.hpp"
#include "xbuffer.hpp"
#include "xmanager.hpp"

namespace xcpp
{
    class XEUS_CLING_API interpreter : public xeus::xinterpreter
    {
    public:

        interpreter(int argc, const char* const* argv);
        virtual ~interpreter();

        void publish_stdout(const std::string&);
        void publish_stderr(const std::string&);

    private:

        void configure_impl() override;

        xeus::xjson execute_request_impl(int execution_counter,
                                         const std::string& code,
                                         bool silent,
                                         bool store_history,
                                         xeus::xjson user_expressions,
                                         bool allow_stdin) override;

        xeus::xjson complete_request_impl(const std::string& code,
                                          int cursor_pos) override;

        xeus::xjson inspect_request_impl(const std::string& code,
                                         int cursor_pos,
                                         int detail_level) override;

        xeus::xjson is_complete_request_impl(const std::string& code) override;

        xeus::xjson kernel_info_request_impl() override;

        void shutdown_request_impl() override;

        xeus::xjson get_error_reply(const std::string& ename,
                                    const std::string& evalue,
                                    const std::vector<std::string>& trace_back);

        void redirect_output();
        void restore_output();

        void init_preamble();
        void init_magic();

        std::string get_stdopt(int argc, const char* const* argv);

        cling::Interpreter m_cling;
        cling::MetaProcessor m_processor;
        std::string m_version;

        xmagics_manager xmagics;
        xpreamble_manager preamble_manager;

        std::streambuf* p_cout_strbuf;
        std::streambuf* p_cerr_strbuf;

        xoutput_buffer m_cout_buffer;
        xoutput_buffer m_cerr_buffer;
    };
}

#endif
