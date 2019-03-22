/***************************************************************************
* Copyright (c) 2018, Martin Renou, Johan Mabille, Sylvain Corlay and      *
* Wolf Vollprecht                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XCPP_INPUT_HPP
#define XCPP_INPUT_HPP

#include <streambuf>

#include "xeus-cling/xbuffer.hpp"

namespace xcpp
{
    /**
     * Input_redirection is a scope guard implementing the redirection of
     * std::cin() to the frontend through an input_request message.
     */
    class input_redirection
    {
    public:

        input_redirection(bool allow_stdin);
        ~input_redirection();

    private:

        std::streambuf* p_cin_strbuf;
        xinput_buffer m_cin_buffer;
    };
}

#endif
