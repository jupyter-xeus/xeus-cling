/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#include <iostream>
#include <stdexcept>
#include <string>

#include "xinput.hpp"

#include "xeus/xinterpreter.hpp"
#include "xeus/xinput.hpp"

namespace xcpp
{
    void notimplemented(const std::string&)
    {
        throw std::runtime_error("This frontend does not support input requests");
    }

    /***************************************
     * Implementation of input_redirection *
     ***************************************/

    input_redirection::input_redirection(bool allow_stdin)
	: p_cin_strbuf(std::cin.rdbuf()),
          m_cin_buffer(allow_stdin ? xinput_buffer([](std::string& value) {
              value = xeus::blocking_input_request("", false);
          }) : xinput_buffer(notimplemented))
    {
        std::cin.rdbuf(&m_cin_buffer);
    }

    input_redirection::~input_redirection()
    {
        std::cin.rdbuf(p_cin_strbuf);
    }
}
