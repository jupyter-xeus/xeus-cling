#ifndef XMESSAGING_BUFFER_HPP
#define XMESSAGING_BUFFER_HPP

#include <streambuf>
#include <functional>
#include <string>
#include <memory>

namespace xeus
{

    class xbuffer : public std::streambuf
    {
    public:

        using callback_type = std::function<void(std::string)>;
   
        xbuffer(callback_type callback) : m_callback(std::move(callback))
        {
            this->setp(this->m_buffer, this->m_buffer + sizeof(this->m_buffer) - 1);
        }

    private:

        int overflow(int c) override
        {
            using traits_type = std::streambuf::traits_type;
            if (!traits_type::eq_int_type(c, traits_type::eof()))
            {
                *this->pptr() = traits_type::to_char_type(c);
                this->pbump(1);
            }
            return this->sync() ? traits_type::not_eof(c): traits_type::eof();
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

}

#endif
