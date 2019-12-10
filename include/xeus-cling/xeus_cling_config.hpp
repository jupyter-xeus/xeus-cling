/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_CLING_CONFIG_HPP
#define XEUS_CLING_CONFIG_HPP

// Project version
#define XEUS_CLING_VERSION_MAJOR 0
#define XEUS_CLING_VERSION_MINOR 8
#define XEUS_CLING_VERSION_PATCH 0

// Composing the version string from major, minor and patch
#define XEUS_CLING_CONCATENATE(A, B) XEUS_CLING_CONCATENATE_IMPL(A, B)
#define XEUS_CLING_CONCATENATE_IMPL(A, B) A##B
#define XEUS_CLING_STRINGIFY(a) XEUS_CLING_STRINGIFY_IMPL(a)
#define XEUS_CLING_STRINGIFY_IMPL(a) #a

#define XEUS_CLING_VERSION XEUS_CLING_STRINGIFY(XEUS_CLING_CONCATENATE(XEUS_CLING_VERSION_MAJOR,   \
                       XEUS_CLING_CONCATENATE(.,XEUS_CLING_CONCATENATE(XEUS_CLING_VERSION_MINOR,   \
                                              XEUS_CLING_CONCATENATE(.,XEUS_CLING_VERSION_PATCH)))))

#ifdef _WIN32
    #ifdef XEUS_CLING_EXPORTS
        #define XEUS_CLING_API __declspec(dllexport)
    #else
        #define XEUS_CLING_API __declspec(dllimport)
    #endif
#else
    #define XEUS_CLING_API
#endif

#endif
