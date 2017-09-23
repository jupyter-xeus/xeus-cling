/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XCPP_MESSAGING_BUFFER_HPP
#define XCPP_MESSAGING_BUFFER_HPP

#include <functional>
#include <memory>
#include <streambuf>
#include <string>

namespace xeus
{

    class xbuffer : public std::streambuf
    {
    public:

        using callback_type = std::function<void(std::string)>;

        xbuffer(callback_type callback)
            : m_callback(std::move(callback))
        {
            this->setp(this->m_buffer, this->m_buffer + sizeof(this->m_buffer) - 1);
        }

    private:

        int overflow(int c) override
        {
            this->sync();          
            using traits_type = std::streambuf::traits_type;
            if (!traits_type::eq_int_type(c, traits_type::eof()))
            {
                *this->pptr() = traits_type::to_char_type(c);
                this->pbump(1);
            }
            return traits_type::not_eof(c);
        }

        int sync() override
        {
            if (this->pbase() != this->pptr())
            {
                this->m_callback(std::string(this->pbase(), this->pptr()));
                this->setp(this->pbase(), this->epptr());
            }
            return 0;
        }

        callback_type m_callback;
        char m_buffer[1024];
    };

    class xnull : public std::streambuf
    {
        int overflow(int c) override
        {
            return c;
        }
    };
}

#endif
