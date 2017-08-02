/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/
#include <string>

#include "cling/MetaProcessor/MetaProcessor.h"

#include "../xmagics.hpp"

namespace xeus
{
    class timeit: public xmagic_line_cell
    {
    public:
        timeit(cling::MetaProcessor* p);
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
    private:
        cling::MetaProcessor *m_processor;

        std::string inner(std::size_t number, std::string const & code) const;
        std::string _format_time(double timespan, std::size_t precision) const;
        void execute(std::string & options, std::string & code) const;
    };
}