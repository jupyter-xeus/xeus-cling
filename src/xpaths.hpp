/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XPYT_PATHS_HPP
#define XPYT_PATHS_HPP

#include <string>

namespace xcpp
{
    /*******************
     * executable_path *
     *******************/

    std::string executable_path();
    
    /*******************
     * prefix_path *
     *******************/

    std::string prefix_path();
}
#endif
