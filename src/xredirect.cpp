/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#include <iostream>
#include <vector>
#include <stdarg.h>

bool c_io_redirect = false;

int redirect_output(FILE * __restrict stream, const char *__restrict format, va_list args)
{
    // Determine the length of the formatted string
    va_list args_length;
    va_copy(args_length, args);
    const int length = vsnprintf(nullptr, 0, format, args_length);
    va_end(args_length);
    const int buffer_size = length + 1;
    std::vector<char> buffer(buffer_size);
    vsnprintf(buffer.data(), buffer_size, format, args);
    va_end(args);

    if (stream == stdout)
    {
        std::cout << buffer.data();
	return length;
    }
    else if (stream == stderr)
    {
        std::cerr << buffer.data();
        return length;
    }
    else
    {
        return vfprintf(stream, format, args);
    }
}

int printf ( const char *__restrict format, ... )
{
    va_list args;
    va_start(args, format);

    if (c_io_redirect)
    {
        return redirect_output(stdout, format, args);
    }
    else
    {
        return vprintf(format, args);
    }
}

int fprintf(FILE *__restrict stream, const char *__restrict format, ...)
{
    va_list args;
    va_start(args, format);

    if (c_io_redirect)
    {
        return redirect_output(stream, format, args);
    }
    else
    {
        return vfprintf(stream,format, args);
    }
}
