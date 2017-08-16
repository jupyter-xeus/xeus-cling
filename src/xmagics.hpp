/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/
#ifndef XMAGICS_HPP
#define XMAGICS_HPP

#include <map>
#include <memory>

#include "xoptions.hpp"

namespace xeus
{
    enum struct xmagic_type{cell, line};

    struct xmagic_line
    {
        virtual void operator()(const std::string& line) = 0;
    };
    
    struct xmagic_cell
    {
        virtual void operator()(const std::string& line, const std::string& cell) = 0;
    };

    struct xmagic_line_cell: public xmagic_line, xmagic_cell
    {
    };

    class xmagics_manager 
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

        bool contains(const std::string& magic_name, const xmagic_type type = xmagic_type::cell)
        {
            if (type == xmagic_type::cell) 
                return m_magic_cell.find(magic_name) != m_magic_cell.end();
            if (type == xmagic_type::line) 
                return m_magic_line.find(magic_name) != m_magic_line.end();
        }

        void apply(const std::string& magic_name, const std::string& line, const std::string& cell)
        {
            if (cell.empty())
            {
                std::cerr << "UsageError: %%" << magic_name << " is a cell magic, but the cell body is empty.";
                if (contains(magic_name, xmagic_type::line))
                    std::cerr << " Did you mean the line magic %" << magic_name << " (single %)?";
                std::cerr << "\n";
                return;
            }
            try
            {
                (*m_magic_cell[magic_name])(line, cell); 
            }
            catch (const cxxopts::OptionException& e)
            {
                std::cerr << "UsageError: " << e.what() << "\n";
            }
            catch (...)
            {
                std::cerr << "Exception occurred. Recovering...\n";
            }
        }

        void apply(const std::string& magic_name, const std::string& line)
        {
            try
            {
                (*m_magic_line[magic_name])(line); 
            }
            catch (const cxxopts::OptionException& e)
            {
                std::cerr << "UsageError: " << e.what() << "\n";
            }
            catch (...)
            {
                std::cerr << "Exception occurred. Recovering...\n";
            }
        }        

    private:
        std::map<std::string, std::shared_ptr<xmagic_cell>> m_magic_cell;
        std::map<std::string, std::shared_ptr<xmagic_line>> m_magic_line;
    };
}
#endif