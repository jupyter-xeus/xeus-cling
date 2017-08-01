/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/
#ifndef XMAGICS_HPP
#define XMAGICS_HPP

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <regex>
#include <type_traits>

#include "cling/Interpreter/Value.h"
#include "cling/Interpreter/Exception.h"
#include "cling/Utils/Output.h"

#include "xparser.hpp"

namespace xeus
{
    struct XEUS_API xmagic_line
    {
        using type = xmagic_line;
        virtual void operator()(const std::string& line) const = 0;
    };
    
    struct XEUS_API xmagic_cell
    {
        virtual void operator()(const std::string& line, const std::string& cell) const = 0;
    };

    struct XEUS_API xmagic_line_cell: public xmagic_line, xmagic_cell
    {
    };

    class XEUS_API xmagics_manager 
    {
    public:

        xmagics_manager(){};

        template<typename xmagic_type>
        void register_magic(const std::string& magic_name, std::shared_ptr<xmagic_type> magic)
        {
            if (std::is_base_of<xmagic_line, xmagic_type>::value)
                m_magic_line[magic_name] = std::dynamic_pointer_cast<xmagic_line>(magic);
            if (std::is_base_of<xmagic_cell, xmagic_type>::value)
                m_magic_cell[magic_name] = std::dynamic_pointer_cast<xmagic_cell>(magic);
        }

        void unregister_magic(const std::string& magic_name)
        {
            m_magic_cell.erase(magic_name);
            m_magic_line.erase(magic_name);
        }

        bool contains(const std::string& magic_name)
        {
            return m_magic_cell.find(magic_name) != m_magic_cell.end();
        }

        void apply(const std::string& magic_name, const std::string& line, const std::string& cell)
        {
            (*m_magic_cell[magic_name])(line, cell); 
        }

        void apply(const std::string& magic_name, const std::string& line)
        {
            (*m_magic_line[magic_name])(line); 
        }

    private:
        std::map<std::string, std::shared_ptr<xmagic_cell>> m_magic_cell;
        std::map<std::string, std::shared_ptr<xmagic_line>> m_magic_line;
    };

    struct XEUS_API writefile: public xmagic_cell
    {
        virtual void operator()(const std::string& line, const std::string& cell) const override
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

        static bool is_file_exist(const char* fileName)
        {
            std::ifstream infile(fileName);
            return infile.good();
        }
    };

    struct XEUS_API timeit: public xmagic_line_cell
    {
        cling::MetaProcessor *m_processor;

        timeit(cling::MetaProcessor* p):m_processor(p){
            cling::Value result;
            cling::Interpreter::CompilationResult compilation_result;

            m_processor->process("#include <chrono>", compilation_result, &result);

            std::string init_timeit = "auto _t0 = std::chrono::high_resolution_clock::now();\n";
            init_timeit += "auto _t1 = std::chrono::high_resolution_clock::now();\n";           
            m_processor->process(init_timeit.c_str(), compilation_result, &result);

        };

        virtual void operator()(const std::string& line) const override
        {
            std::string cline = line;
            execute(cline, cline);
        }

        virtual void operator()(const std::string& line, const std::string& cell) const override
        {
            std::string cline = line;
            std::string ccell = cell;
            execute(cline, ccell);
        }

        std::string inner(std::size_t number, std::string const & code) const
        {
            std::string timeit_code = "";
            timeit_code += "_t0 = std::chrono::high_resolution_clock::now();\n";
            timeit_code += "for(std::size_t _i=0; _i<" + std::to_string(number) + ";++_i){\n";
            timeit_code += "   " + code + "\n";
            timeit_code += "}\n";
            timeit_code += "_t1 = std::chrono::high_resolution_clock::now();\n";
            timeit_code += "std::chrono::duration<double>(_t1-_t0).count();";
            return timeit_code;
        }

        std::string _format_time(double timespan, std::size_t precision) const
        {
            std::vector<std::string> units{"s", "ms", "us", "ns"};
            std::vector<double> scaling{1, 1e3, 1e6, 1e9};
            std::ostringstream output;
            int order;

            if (timespan > 0.0)
                order = std::min(-static_cast<int>(std::floor(std::floor(std::log10(timespan))/3)), 3);
            else
                order = 3;
            output.precision(precision);
            output << timespan * scaling[order] << " " << units[order];
            return output.str();
        }

        void execute(std::string & options, std::string & code) const
        {
            auto opts = parse_opts(options, "n:r:p:qo");

            std::size_t number = (opts.find("n") != opts.end())? std::atoi(opts["n"].c_str()): 0ul;
            std::size_t default_repeat = 7;
            std::size_t repeat = (opts.find("r") != opts.end())? std::atoi(opts["r"].c_str()): default_repeat;
            std::size_t precision = (opts.find("p") != opts.end())? std::atoi(opts["p"].c_str()): 3;
            bool quiet = (opts.find("q") != opts.end());
            bool return_result = (opts.find("o") != opts.end());

            if (trim(code).empty())
                return;

            cling::Value result;
            cling::Interpreter::CompilationResult compilation_result;

            auto errorlevel = 0;
            try
            {
                if (number == 0)
                    for(std::size_t n=0; n<10; ++n)
                    {
                        number = std::pow(10, n);
                        std::string timeit_code = inner(number, code);
                        errorlevel = m_processor->process(timeit_code.c_str(), compilation_result, &result);
                        if (result.simplisticCastAs<double>() >= 0.2)
                            break;
                    }

                std::vector<double> all_runs;
                double mean = 0;
                double stdev = 0;
                for(std::size_t r=0; r<repeat; ++r)
                {
                    std::string timeit_code = inner(number, code);
                    errorlevel = m_processor->process(timeit_code.c_str(), compilation_result, &result);
                    all_runs.push_back(result.simplisticCastAs<double>()/number);
                    mean += all_runs.back();
                }
                mean /= repeat;
                for(std::size_t r=0; r<repeat; ++r)
                    stdev += (all_runs[r] - mean)*(all_runs[r] - mean);
                stdev = std::sqrt(stdev/repeat);
                auto minmax = std::minmax_element(all_runs.begin(), all_runs.end());
                std::cout << _format_time(mean, precision) << " +- " << _format_time(stdev, precision);
                std::cout << " per loop (mean +- std. dev. of " << repeat << " run" << ((repeat==1)? ", ":"s ");
                std::cout << number << " loop" << ((number==1)? "":"s") << " each)\n";             
            }
            catch (cling::InterpreterException& e)
            {
                if (!e.diagnose())
                {
                    std::cerr << "Caught an interpreter exception!\n"
                           << e.what() << '\n';
                }
            }
            catch (std::exception& e)
            {
                std::cerr << "Caught a std::exception!\n"
                       << e.what() << '\n';
            }
            catch (...)
            {
                std::cerr << "Exception occurred. Recovering...\n";
            }
        }
    };
}
#endif