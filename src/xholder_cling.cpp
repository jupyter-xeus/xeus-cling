#include "xholder_cling.hpp"
#include "xpreamble.hpp"

#include <algorithm>

namespace xeus
{

    /***********************************
     * xholder_preamble implementation *
     ***********************************/
    xholder_preamble::xholder_preamble()
        : p_holder(nullptr)
    {
    }

    xholder_preamble::xholder_preamble(xpreamble* holder)
        : p_holder(holder)
    {
    }

    xholder_preamble::~xholder_preamble()
    {
        delete p_holder;
    }

    xholder_preamble::xholder_preamble(const xholder_preamble& rhs)
        : p_holder(rhs.p_holder ? rhs.p_holder->clone() : nullptr)
    {
    }

    xholder_preamble::xholder_preamble(xholder_preamble&& rhs)
        : p_holder(rhs.p_holder)
    {
        rhs.p_holder = nullptr;
    }

    xholder_preamble& xholder_preamble::operator=(const xholder_preamble& rhs)
    {
        xholder_preamble tmp(rhs);
        swap(tmp);
        return *this;
    }

    xholder_preamble& xholder_preamble::operator=(xholder_preamble&& rhs)
    {
        xholder_preamble tmp(std::move(rhs));
        swap(tmp);
        return *this;
    }

    xholder_preamble& xholder_preamble::operator=(xpreamble* holder)
    {
        delete p_holder;
        p_holder = holder;
        return *this;
    }


    void xholder_preamble::apply(const std::string& s, xjson& kernel_res)
    {
        if (p_holder != nullptr)
        {
            p_holder->apply(s, kernel_res);
        }
    }

    bool xholder_preamble::is_match(const std::string& s) const
    {
        if (p_holder != nullptr)
        {
            return p_holder->is_match(s);
        }
        return false;
    }
}