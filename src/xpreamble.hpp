/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/
#ifndef XPREAMBLE_HPP
#define XPREAMBLE_HPP

#include "xeus/xjson.hpp"

namespace xeus{
    struct xpreamble
    {
    std::regex pattern;

    bool is_match(const std::string& s) const
    {
        std::smatch match;
        return std::regex_search(s, match, pattern);
    }

    void set_pattern(const std::string r)
    {
        pattern = r;
    }

    virtual void apply(const std::string& s, xjson& kernel_res) = 0;

    };

    struct xpreamble_manager
    {
        std::map<std::string, std::unique_ptr<xpreamble>> preamble;

        template<typename preamble_type>
        void register_preamble(const std::string& name, const preamble_type& pre)
        {
            auto ptr = std::make_unique<preamble_type>(pre);
            preamble[name] = std::move(ptr);
        }

        template<typename preamble_type>
        void unregister_preamble(const std::string& name)
        {
            preamble.erase(name);
        }

    };
}
#endif
