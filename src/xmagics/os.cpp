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
    xoptions writefile::get_options()
    {
        xoptions options{"file", "write file"};
        options.add_options()
            ("a,append", "append")
            ("f,filename", "filename", cxxopts::value<std::string>());
        options.parse_positional("filename");
        return options;
    }

    void writefile::operator()(const std::string& line, const std::string& cell)
    {
        auto options = get_options();
        auto result = options.parse(line);

        auto append = result.count("a");
        auto filename = result["filename"].as<std::string>();
        if (filename.empty())
        {
            std::cerr << "UsageError: the following arguments are required: filename\n";
            return;
        }

        std::ofstream file;

        // TODO: check permission rights
        if (is_file_exist(filename.c_str()))
        {
            if (append)
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
