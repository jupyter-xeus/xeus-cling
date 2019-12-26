/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XCPP_MESSAGING_BUFFER_HPP
#define XCPP_MESSAGING_BUFFER_HPP

#include <functional>
#include <memory>
#include <streambuf>
#include <string>

namespace xcpp
{
    /********************
     * output streambuf *
     ********************/

    class xoutput_buffer : public std::streambuf
    {
    public:

        using base_type = std::streambuf;
        using callback_type = std::function<void(std::string)>;
        using traits_type = base_type::traits_type;

        xoutput_buffer(callback_type callback)
            : m_callback(std::move(callback))
        {
        }

    protected:

        traits_type::int_type overflow(traits_type::int_type c) override
        {
            // Called for each output character.
            if (!traits_type::eq_int_type(c, traits_type::eof()))
            {
                m_output.push_back(traits_type::to_char_type(c));
            }
            return c;
        }

        traits_type::int_type sync() override
        {
            // Called in case of flush.
            if (!m_output.empty())
            {
                m_callback(m_output);
                m_output.clear();
            }
            return 0;
        }

        callback_type m_callback;
        std::string m_output;
    };

    /*******************
     * input streambuf *
     *******************/

    class xinput_buffer : public std::streambuf
    {
    public:

        using base_type = std::streambuf;
        using callback_type = std::function<void(std::string&)>;
        using traits_type = base_type::traits_type;

        xinput_buffer(callback_type callback)
            : m_callback(std::move(callback))
            , m_value()
        {
            char* data = const_cast<char*>(m_value.data());
            this->setg(data, data, data);
        }

    protected:

        traits_type::int_type underflow() override
        {
            m_callback(m_value);
            // Terminate the string to trigger parsing.
            m_value += '\n';
            char* data = const_cast<char*>(m_value.data());
            setg(data, data, data + m_value.size());
            return traits_type::to_int_type(*gptr());
        }

        callback_type m_callback;
        std::string m_value;
    };

    /*************************
     * output null streambuf *
     *************************/

    class xnull : public std::streambuf
    {
        using base_type = std::streambuf;
        using traits_type = base_type::traits_type;

        traits_type::int_type overflow(traits_type::int_type c) override
        {
            return c;
        }
    };
}

#endif
