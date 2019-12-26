/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XCPP_HOLDER_CLING_HPP
#define XCPP_HOLDER_CLING_HPP

#include <regex>

#include "nlohmann/json.hpp"

#include "xeus-cling/xpreamble.hpp"

namespace nl = nlohmann;

namespace xcpp
{
    class xholder_preamble
    {
    public:

        xholder_preamble();
        ~xholder_preamble();
        xholder_preamble(const xholder_preamble& rhs);
        xholder_preamble(xholder_preamble&& rhs);
        xholder_preamble(xpreamble* holder);

        xholder_preamble& operator=(const xholder_preamble& rhs);
        xholder_preamble& operator=(xholder_preamble&& rhs);

        xholder_preamble& operator=(xpreamble* holder);

        void swap(xholder_preamble& rhs)
        {
            std::swap(p_holder, rhs.p_holder);
        }

        void apply(const std::string& s, nl::json& kernel_res);
        bool is_match(const std::string& s) const;

        template <class D>
        D& get_cast()
        {
            return dynamic_cast<D&>(*p_holder);
        }

    private:

        xpreamble* p_holder;
    };
}
#endif
