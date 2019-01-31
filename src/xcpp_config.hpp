/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XCPP_CONFIG_HPP
#define XCPP_CONFIG_HPP

// Project version
#define XCPP_VERSION_MAJOR 0
#define XCPP_VERSION_MINOR 4
#define XCPP_VERSION_PATCH 11

// Composing the version string from major, minor and patch
#define XCPP_CONCATENATE(A, B) XCPP_CONCATENATE_IMPL(A, B)
#define XCPP_CONCATENATE_IMPL(A, B) A##B
#define XCPP_STRINGIFY(a) XCPP_STRINGIFY_IMPL(a)
#define XCPP_STRINGIFY_IMPL(a) #a

#define XCPP_VERSION XCPP_STRINGIFY(XCPP_CONCATENATE(XCPP_VERSION_MAJOR,   \
                 XCPP_CONCATENATE(.,XCPP_CONCATENATE(XCPP_VERSION_MINOR,   \
                                  XCPP_CONCATENATE(.,XCPP_VERSION_PATCH)))))

#endif
