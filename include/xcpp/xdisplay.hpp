/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XCPP_DISPLAY_HPP
#define XCPP_DISPLAY_HPP

#include "nlohmann/json.hpp"

#include "xmime.hpp"

namespace nl = nlohmann;

namespace xcpp
{
    template <class T>
    void display(const T& t)
    {
        using ::xcpp::mime_bundle_repr;
        xeus::get_interpreter().display_data(
            mime_bundle_repr(t),
            nl::json::object(),
            nl::json::object()
        );
    }

    template <class T>
    void display(const T& t, xeus::xguid id, bool update=false)
    {
        nl::json transient;
        transient["display_id"] = id;
        using ::xcpp::mime_bundle_repr;
        if (update)
        {
            xeus::get_interpreter().update_display_data(
                mime_bundle_repr(t),
                nl::json::object(),
                std::move(transient)
            );
        }
        else
        {
            xeus::get_interpreter().display_data(
                mime_bundle_repr(t),
                nl::json::object(),
                std::move(transient)
            );
        }
    }

    inline void clear_output(bool wait=false)
    {
        xeus::get_interpreter().clear_output(wait);
    }
}

#endif
