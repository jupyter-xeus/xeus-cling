/***************************************************************************
* Copyright (c) 2018, Martin Renou, Johan Mabille, Sylvain Corlay and      *
* Wolf Vollprecht                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <iostream>
#include <stdexcept>
#include <string>

#include "xinput.hpp"

#include "xeus/xinterpreter.hpp"

namespace xcpp
{
    std::string input_request(const std::string& prompt, bool password)
    {
        auto& interpreter = xeus::get_interpreter();

        // Register the input handler
        std::string value;
        interpreter.register_input_handler([&value](const std::string& v) {
            value = v;
        });

        // Send the input request
        interpreter.input_request(prompt, password);

        // Remove input handler
        interpreter.register_input_handler(nullptr);

        return value;
    }

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
              value = input_request("", false);
          }) : xinput_buffer(notimplemented))
    {
        std::cin.rdbuf(&m_cin_buffer);
    }

    input_redirection::~input_redirection()
    {
        std::cin.rdbuf(p_cin_strbuf);
    }
}
