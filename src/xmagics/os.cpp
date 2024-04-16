/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "xeus-cling/xoptions.hpp"

#include "../xparser.hpp"

#include "os.hpp"

namespace xcpp
{
    static void get_options(argparser &argpars)
    {
        argpars.add_description("write file");
        argpars.add_argument("-a", "--append")
            .help("append")
            .default_value(false)
            .implicit_value(true);
        argpars.add_argument("filename")
            .help("filename")
            .required();
        // Add custom help (does not call `exit` avoiding to restart the kernel)
        argpars.add_argument("-h", "--help")
            .action([&](const std::string & /*unused*/)
            {
                std::cout << argpars.help().str();
            })
            .default_value(false)
            .help("shows help message")
            .implicit_value(true)
            .nargs(0);
    }

    void writefile::operator()(const std::string& line, const std::string& cell)
    {
        argparser argpars("file", XEUS_CLING_VERSION, argparse::default_arguments::none);
        get_options(argpars);
        argpars.parse(line);

        auto filename = argpars.get<std::string>("filename");

        std::ofstream file;

        // TODO: check permission rights
        if (is_file_exist(filename.c_str()))
        {
            if (argpars["-a"] == true)
            {
                file.open(filename, std::ios::app);
                std::cout << "Appending to " << filename << "\n";
            }
            else
            {
                file.open(filename);
                std::cout << "Overwriting " << filename << "\n";
            }
        }
        else
        {
            file.open(filename);
            std::cout << "Writing " << filename << "\n";
        }
        file << cell << "\n";
        file.close();
    }

    bool writefile::is_file_exist(const char* fileName)
    {
        std::ifstream infile(fileName);
        return infile.good();
    }
}
