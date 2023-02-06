/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XCPP_MIME_HPP
#define XCPP_MIME_HPP

#include <complex>
#include <sstream>
#include <type_traits>

#include "nlohmann/json.hpp"

#include "cling/Interpreter/RuntimePrintValue.h"

namespace nl = nlohmann;

namespace xcpp
{

    namespace detail
    {

        // Generic mime_bundle_repr() implementation
        // via std::ostringstream.
        template <class T>
        nl::json mime_bundle_repr_via_sstream(const T& value)
        {
            auto bundle = nl::json::object();

            std::ostringstream oss;
            oss << value;

            bundle["text/plain"] = oss.str();
            return bundle;
        }

        // Helper struct for SFINAE concept check - obsolete with C++17
        template<class...>
        using void_t = void;
    }

    // Default implementation of mime_bundle_repr
    template <class T>
    nl::json fallback_mime_bundle_repr(const T& value)
    {
        auto bundle = nl::json::object();
        bundle["text/plain"] = cling::printValue(&value);
        return bundle;
    }

    // Implementation for std::complex.
    template <class T>
    nl::json fallback_mime_bundle_repr(const std::complex<T>& value)
    {
        return detail::mime_bundle_repr_via_sstream(value);
    }

    // Implementation for long double. This is a workaround for
    // https://github.com/jupyter-xeus/xeus-cling/issues/220
    inline nl::json fallback_mime_bundle_repr(const long double& value)
    {
        return detail::mime_bundle_repr_via_sstream(value);
    }

    template<typename T, typename = void>
    struct MimeBundleReprDispatcher
    {
        template<typename U>
        static nl::json dispatch(U&& u) {
            return fallback_mime_bundle_repr(std::forward<U>(u));
        }
    };

    template<typename T>
    struct MimeBundleReprDispatcher<T, detail::void_t<decltype(mime_bundle_repr(std::declval<T>()))>>
    {
        template<typename U>
        static nl::json dispatch(U&& u) {
            return mime_bundle_repr(std::forward<U>(u));
        }
    };

    template<typename T>
    nl::json dispatch_mime_bundle_repr(T&& t)
    {
        return MimeBundleReprDispatcher<T>::dispatch(std::forward<T>(t));
    }
}

#endif
