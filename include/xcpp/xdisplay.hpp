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
    // concept check wheather T has a mime_bundle_repr overload
    // defalut case: mime_bundle_repr overload does fail
    template<class T, class = void>
    struct has_mime_bundle_repr
        : public std::false_type
    {};

    // specialized case (SFINAE): mime_bundle_repr overload does not fail
    template<class T>
    struct has_mime_bundle_repr<T,std::void_t<decltype(mime_bundle_repr(std::declval<T>()))>>
        : std::true_type
    {};

    template<class T>
    nl::json dispatch_bundle_repr(T&& t) {
        if constexpr (has_mime_bundle_repr<T>{})
            return mime_bundle_repr(std::forward<T>(t));
        else
            return fallback_mime_bundle_repr(std::forward<T>(t));
    }
    
    template <class T>
    void display(T&& t)
    {
        xeus::get_interpreter().display_data(
            dispatch_bundle_repr(std::forward<T>(t)),
            nl::json::object(),
            nl::json::object()
        );
    }

    template <class T>
    void display(T&& t, xeus::xguid id, bool update=false)
    {
        nl::json transient;
        transient["display_id"] = id;
        if (update)
        {
            xeus::get_interpreter().update_display_data(
                dispatch_bundle_repr(std::forward<T>(t)),
                nl::json::object(),
                std::move(transient)
            );
        }
        else
        {
            xeus::get_interpreter().display_data(
                dispatch_bundle_repr(std::forward<T>(t)),
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
