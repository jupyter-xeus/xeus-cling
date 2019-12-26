/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XCPP_DEMANGLE_HPP
#define XCPP_DEMANGLE_HPP

// __has_include is currently supported by GCC and Clang. However GCC 4.9 may have issues and
// returns 1 for 'defined( __has_include )', while '__has_include' is actually not supported:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=63662
#if defined(__has_include) && (!defined(__GNUC__) || (__GNUC__ + 0) >= 5)
#if __has_include(<cxxabi.h>)
#define XEUS_HAS_CXXABI_H
#endif
#elif defined(__GLIBCXX__) || defined(__GLIBCPP__)
#define XEUS_HAS_CXXABI_H
#endif

#if defined(XEUS_HAS_CXXABI_H)
#include <cxxabi.h>
// For some archtectures (mips, mips64, x86, x86_64) cxxabi.h in Android NDK is implemented by gabi++ library
// (https://android.googlesource.com/platform/ndk/+/master/sources/cxx-stl/gabi++/), which does not implement
// abi::__cxa_demangle(). We detect this implementation by checking the include guard here.
#if defined(_GABIXX_CXXABI_H__)
#undef XEUS_HAS_CXXABI_H
#else
#include <cstdlib>
#endif
#endif

namespace xcpp
{
    const char* demangle(const char* name) noexcept;
    const char* demangle(const std::string& name) noexcept;

#if defined(XEUS_HAS_CXXABI_H)

    inline const char* demangle(const char* name) noexcept
    {
        std::size_t size = 0;
        int status = 0;
        return abi::__cxa_demangle(name, 0, &size, &status);
    }

    inline const char* demangle(const std::string& name) noexcept
    {
        std::size_t size = 0;
        int status = 0;
        return abi::__cxa_demangle(name.c_str(), 0, &size, &status);
    }

#else

    inline const char* demangle(const char* name) noexcept
    {
        return name;
    }

    inline const char* demangle(const std::string& name) noexcept
    {
        return demangle(name.c_str());
    }

#endif
}

#endif
