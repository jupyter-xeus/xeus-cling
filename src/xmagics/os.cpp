/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/
#include <iostream>
#include <fstream>
#include <string>

#include "os.hpp"
#include "../xparser.hpp"

namespace xeus
{
    void writefile::operator()(const std::string& line, const std::string& cell) const
    {
        std::string cline = line;
        auto opts = parse_opts(cline, "a");
        auto it = opts.find("a");
        auto filename = trim(cline);
        std::ofstream file;

        // TODO: check permission rights
        if (is_file_exist(filename.c_str()))
        {
            if (it != opts.end())
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