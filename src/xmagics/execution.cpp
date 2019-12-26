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

#include "cling/Interpreter/Value.h"
#include "cling/Interpreter/Exception.h"
#include "cling/MetaProcessor/MetaProcessor.h"
#include "cling/Utils/Output.h"

#include "execution.hpp"
#include "../xparser.hpp"

namespace xcpp
{
    timeit::timeit(cling::MetaProcessor* p)
        : m_processor(p)
    {
        cling::Interpreter::CompilationResult compilation_result;

        m_processor->process("#include <chrono>", compilation_result);

        std::string init_timeit = "auto _t0 = std::chrono::high_resolution_clock::now();\n";
        init_timeit += "auto _t1 = std::chrono::high_resolution_clock::now();\n";
        m_processor->process(init_timeit.c_str(), compilation_result);
    }

    xoptions timeit::get_options()
    {
        xoptions options{"timeit", "Time execution of a C++ statement or expression"};
        options.add_options()
            ("n,number", "execute the given statement n times in a loop. If this value is not given, a fitting value is chosen", cxxopts::value<std::size_t>())
            ("r,repeat", "repeat the loop iteration r times and take the best result", cxxopts::value<std::size_t>()->default_value("7"))
            ("p,precision", "use a precision of p digits to display the timing result.", cxxopts::value<std::size_t>()->default_value("3"))
            ("positional",
             "Positional arguments: these are the arguments that are entered "
             "without an option", cxxopts::value<std::vector<std::string>>());
        options.parse_positional("positional");
        return options;
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

        auto options = get_options();
        auto result = options.parse(line);

        std::size_t number = (result.count("n")) ? result["n"].as<std::size_t>() : 0ul;
        std::size_t repeat = result["r"].as<std::size_t>();
        std::size_t precision = result["p"].as<std::size_t>();

        std::string code;
        if (result.count("positional"))
        {
            auto& v = result["positional"].as<std::vector<std::string>>();
            for (const auto& s : v)
            {
                code += " " + s;
            }
        }

        code += cell;
        if (trim(code).empty())
        {
            return;
        }

        auto errorlevel = 0;
        auto indent = 0;
        std::string ename;
        std::string evalue;
        cling::Value output;
        cling::Interpreter::CompilationResult compilation_result;

        try
        {
            if (number == 0ul)
            {
                for (std::size_t n = 0; n < 10; ++n)
                {
                    number = std::pow(10, n);
                    std::string timeit_code = inner(number, code);
                    indent = m_processor->process(timeit_code.c_str(), compilation_result, &output);
                    if (output.simplisticCastAs<double>() >= 0.2)
                    {
                        break;
                    }
                }
            }

            std::vector<double> all_runs;
            double mean = 0;
            double stdev = 0;
            for (std::size_t r = 0; r < repeat; ++r)
            {
                std::string timeit_code = inner(number, code);
                indent = m_processor->process(timeit_code.c_str(), compilation_result, &output);
                all_runs.push_back(output.simplisticCastAs<double>() / number);
                mean += all_runs.back();
            }
            mean /= repeat;
            for (std::size_t r = 0; r < repeat; ++r)
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

        // If an error was encountered, don't attempt further execution
        if (errorlevel)
        {
            m_processor->cancelContinuation();
        }
    }
}
