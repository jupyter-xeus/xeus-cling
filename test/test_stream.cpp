/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#include "gtest/gtest.h"

#include <functional>
#include <iostream>
#include <list>
#include <string>

#include "xeus-cling/xbuffer.hpp"

using namespace std::placeholders;

void callback(std::string value, std::list<std::string>& outputs)
{
    outputs.push_back(value);
}

TEST(stream, output)
{
    std::list<std::string> outputs;
    xcpp::xoutput_buffer buffer(std::bind(callback, _1, std::ref(outputs)));
    auto cout_strbuf = std::cout.rdbuf();
    std::cout.rdbuf(&buffer);
    std::cout << "Some output" << std::endl;
    EXPECT_EQ(outputs.front(), "Some output\n");
    std::cout.rdbuf(cout_strbuf);
}
