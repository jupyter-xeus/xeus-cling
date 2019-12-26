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

#include "nlohmann/json.hpp"

#include "cling/Interpreter/RuntimePrintValue.h"

namespace nl = nlohmann;

namespace xcpp
{
    // Default implementation of mime_bundle_repr
    template <class T>
    nl::json mime_bundle_repr(const T& value)
    {
        auto bundle = nl::json::object();
        bundle["text/plain"] = cling::printValue(&value);
        return bundle;
    }
}

#endif
