/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/
#ifndef XMAGICS_HPP
#define XMAGICS_HPP

#include "xparser.hpp"
#include <memory>
#include <regex>

namespace xeus
{

    struct XEUS_API xmagic_base
    {
        auto parse_opts(std::string& line, const std::string& opts) const
        {
            auto map_opts = getopt(line, opts);
            // remove opts found in line
            for (auto o: map_opts)
            {
                std::string tmp = "\\-" + o.first + "\\s*";
                if (!o.second.empty())
                    tmp += o.second;
                line = std::regex_replace(line, std::regex(tmp), "");
            }
            return map_opts;
        }
    };

    struct XEUS_API xmagic_line: public xmagic_base
    {
        virtual void operator()(const std::string& line) const{};
    };
    
    struct XEUS_API xmagic_cell: public xmagic_base
    {
        virtual void operator()(const std::string& line, const std::string& cell) const{};
    };

    class XEUS_API xmagics_manager 
    {
    public:

        xmagics_manager(){};

        void register_magic(const std::string& magic_name, std::shared_ptr<xmagic_line> m_line)
        {
            m_magic_line[magic_name] = m_line;
        }

        void register_magic(const std::string& magic_name, std::shared_ptr<xmagic_cell> m_cell)
        {
            m_magic_cell[magic_name] = m_cell;
        }

        void register_magic(const std::string& magic_name, std::shared_ptr<xmagic_line> m_line, std::shared_ptr<xmagic_cell> m_cell)
        {
            m_magic_line[magic_name] = m_line;
            m_magic_cell[magic_name] = m_cell;
        }

        void unregister_magic(const std::string& magic_name)
        {
            m_magic_cell.erase(magic_name);
            m_magic_line.erase(magic_name);
        }

        bool exist(const std::string& magic_name)
        {
            auto it = m_magic_cell.find(magic_name);
            if (it != m_magic_cell.end())
                return true;
            return false;
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
            if (it != opts.end())
                std::cout << "option a\n";
            std::cout << cline << "\n";
            std::cout << cell << "\n";
        }
    };
    // static inline void writefile(const std::string& line, const std::string& cell)
    // {

    //     std::cout << line << "\n";
    //     std::cout << cell << "\n";
    // }
}
#endif