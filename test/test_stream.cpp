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
