/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XCPP_MIME_HPP
#define XCPP_MIME_HPP

#include "cling/Interpreter/RuntimePrintValue.h"
#include "xeus/xjson.hpp"

namespace xcpp
{
    // Default implementation of mime_bundle_repr
    template <class T>
    xeus::xjson mime_bundle_repr(const T& value)
    {
        auto bundle = xeus::xjson::object();
        bundle["text/plain"] = cling::printValue(&value);
        return bundle;
    }
}

#endif
