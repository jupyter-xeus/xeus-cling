/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

#include "cling/Interpreter/Value.h"
#include "cling/Interpreter/Exception.h"
#include "cling/Interpreter/Interpreter.h"
#include "cling/Utils/Output.h"

#include "execution.hpp"
#include "../xparser.hpp"

namespace xcpp
{
    timeit::timeit(cling::Interpreter* p)
        : m_interpreter(p)
    {
        cling::Interpreter::CompilationResult compilation_result;
        compilation_result = m_interpreter->process("#include <chrono>");
        std::string init_timeit = "auto _t0 = std::chrono::high_resolution_clock::now();\n";
        init_timeit += "auto _t1 = std::chrono::high_resolution_clock::now();\n";
        compilation_result = m_interpreter->process(init_timeit.c_str());
    }

    void timeit::get_options(argparser &argpars)
    {
        argpars.add_description("Time execution of a C++ statement or expression");
        argpars.add_argument("-n", "--number")
            .help("execute the given statement n times in a loop. If this value is not given, a fitting value is chosen")
            .default_value(0)
            .scan<'i', int>();
        argpars.add_argument("-r", "--repeat")
            .help("repeat the loop iteration r times and take the best result")
            .default_value(7)
            .scan<'i', int>();
        argpars.add_argument("-p", "--precision")
            .help("use a precision of p digits to display the timing result")
            .default_value(3)
            .scan<'i', int>();
        argpars.add_argument("expression")
            .help("expression to be evaluated")
            .remaining();
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

    std::string timeit::inner(std::size_t number, const std::string& code) const
    {
        std::string timeit_code = "";
        timeit_code += "_t0 = std::chrono::high_resolution_clock::now();\n";
        timeit_code += "for (std::size_t _i = 0; _i < " + std::to_string(number) + "; ++_i) {\n";
        timeit_code += "   " + code + "\n";
        timeit_code += "}\n";
        timeit_code += "_t1 = std::chrono::high_resolution_clock::now();\n";
        timeit_code += "std::chrono::duration<double>(_t1-_t0).count();";
        return timeit_code;
    }

    std::string timeit::_format_time(double timespan, std::size_t precision) const
    {
        std::vector<std::string> units{"s", "ms", "us", "ns"};
        std::vector<double> scaling{1, 1e3, 1e6, 1e9};
        std::ostringstream output;
        int order;

        if (timespan > 0.0)
        {
            order = std::min(-static_cast<int>(std::floor(std::floor(std::log10(timespan)) / 3)), 3);
        }
        else
        {
            order = 3;
        }
        output.precision(precision);
        output << timespan * scaling[order] << " " << units[order];
        return output.str();
    }

    void timeit::execute(std::string& line, std::string& cell)
    {
        // std::istringstream iss(line);
        // std::vector<std::string> results((std::istream_iterator<std::string>(iss)),
        //                          std::istream_iterator<std::string>());

        argparser argpars("timeit", XEUS_CLING_VERSION, argparse::default_arguments::none);
        get_options(argpars);
        argpars.parse(line);

        // TODO find a way to use std::size_t
        int number = argpars.get<int>("-n");
        int repeat = argpars.get<int>("-r");
        int precision = argpars.get<int>("-p");

        std::string code;
        try
        {
            const auto& v = argpars.get<std::vector<std::string>>("expression");
            for (const auto& s : v)
            {
                code += " " + s;
            }
        }
        catch (std::logic_error& e)
        {
            if (trim(cell).empty() && (argpars["-h"] == false))
            {
                std::cerr << "No expression given to evaluate" << std::endl;
            }
        }

        code += cell;
        if (trim(code).empty())
        {
            return;
        }

        auto errorlevel = 0;
        std::string ename;
        std::string evalue;
        cling::Value output;
        cling::Interpreter::CompilationResult compilation_result = cling::Interpreter::kSuccess;

        try
        {
            if (number == 0)
            {
                for (std::size_t n = 0; n < 10; ++n)
                {
                    number = std::pow(10, n);
                    std::string timeit_code = inner(number, code);
                    compilation_result = m_interpreter->process(timeit_code.c_str(), &output);
                    if (output.castAs<double>() >= 0.2)
                    {
                        break;
                    }
                }
            }

            std::vector<double> all_runs;
            double mean = 0;
            double stdev = 0;
            for (std::size_t r = 0; r < static_cast<std::size_t>(repeat); ++r)
            {
                std::string timeit_code = inner(number, code);
                compilation_result = m_interpreter->process(timeit_code.c_str(), &output);
                all_runs.push_back(output.castAs<double>() / number);
                mean += all_runs.back();
            }
            mean /= repeat;
            for (std::size_t r = 0; r < static_cast<std::size_t>(repeat); ++r)
            {
                stdev += (all_runs[r] - mean) * (all_runs[r] - mean);
            }
            stdev = std::sqrt(stdev / repeat);

            std::cout << _format_time(mean, precision) << " +- " << _format_time(stdev, precision);
            std::cout << " per loop (mean +- std. dev. of " << repeat << " run" << ((repeat == 1) ? ", " : "s ");
            std::cout << number << " loop" << ((number == 1) ? "" : "s") << " each)" << std::endl;
        }
        // Catch all errors
        catch (cling::InterpreterException& e)
        {
            errorlevel = 1;
            ename = "Interpreter Exception";
            if (!e.diagnose())
            {
                evalue = e.what();
            }
        }
        catch (std::exception& e)
        {
            errorlevel = 1;
            ename = "Standard Exception";
            evalue = e.what();
        }
        catch (...)
        {
            errorlevel = 1;
            ename = "Error";
        }

        if (compilation_result != cling::Interpreter::kSuccess)
        {
            errorlevel = 1;
            ename = "Interpreter Error";
        }
    }
}
