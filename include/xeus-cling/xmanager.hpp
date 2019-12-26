/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XCPP_MANAGER_HPP
#define XCPP_MANAGER_HPP

#include <map>
#include <memory>
#include <regex>
#include <string>
#include <type_traits>

#include "nlohmann/json.hpp"

#include "xeus-cling/xholder_cling.hpp"
#include "xeus-cling/xmagics.hpp"
#include "xeus-cling/xpreamble.hpp"

namespace nl = nlohmann;

namespace xcpp
{
    struct xpreamble_manager
    {
        std::map<std::string, xholder_preamble> preamble;

        template <typename preamble_type>
        void register_preamble(const std::string& name, preamble_type* pre)
        {
            preamble[name] = xholder_preamble(pre);
        }

        void unregister_preamble(const std::string& name)
        {
            preamble.erase(name);
        }

        xholder_preamble& operator[](const std::string& name)
        {
            return preamble[name];
        }
    };

    class xmagics_manager : public xpreamble
    {
    public:

        using xpreamble::pattern;

        xmagics_manager()
        {
            pattern = R"(^(?:\%{2}|\%)(\w+))";
        }

        template <typename xmagic_type>
        void register_magic(const std::string& magic_name, xmagic_type magic)
        {
            auto shared = std::make_shared<xmagic_type>(magic);
            if (std::is_base_of<xmagic_line, xmagic_type>::value)
            {
                m_magic_line[magic_name] = std::dynamic_pointer_cast<xmagic_line>(shared);
            }
            if (std::is_base_of<xmagic_cell, xmagic_type>::value)
            {
                m_magic_cell[magic_name] = std::dynamic_pointer_cast<xmagic_cell>(shared);
            }
        }

        void unregister_magic(const std::string& magic_name)
        {
            m_magic_cell.erase(magic_name);
            m_magic_line.erase(magic_name);
        }

        bool contains(const std::string& magic_name, const xmagic_type type = xmagic_type::cell)
        {
            if (type == xmagic_type::cell)
            {
                return m_magic_cell.find(magic_name) != m_magic_cell.end();
            }
            if (type == xmagic_type::line)
            {
                return m_magic_line.find(magic_name) != m_magic_line.end();
            }
            return false;
        }

        void apply(const std::string& magic_name, const std::string& line, const std::string& cell)
        {
            if (cell.empty())
            {
                std::cerr << "UsageError: %%" << magic_name << " is a cell magic, but the cell body is empty.";
                if (contains(magic_name, xmagic_type::line))
                {
                    std::cerr << " Did you mean the line magic %" << magic_name << " (single %)?";
                }
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

        void apply(const std::string& code, nl::json& kernel_res) override
        {
            std::regex re_magic_cell(R"(^\%{2}(\w+))");
            std::smatch magic_name;
            if (std::regex_search(code, magic_name, re_magic_cell))
            {
                if (!contains(magic_name.str(1)))
                {
                    std::cerr << "Unknown magic cell function %%" << magic_name[1] << "\n";
                    std::cout << std::flush;
                    kernel_res["status"] = "error";
                    kernel_res["ename"] = "ename";
                    kernel_res["evalue"] = "evalue";
                    kernel_res["traceback"] = nl::json::array();
                    return;
                }
                std::regex re_magic_cell(R"(^\%{2}(\w+(?:\s.*)?)\n((?:.*\n?)*))");
                std::smatch split_code;
                std::regex_search(code, split_code, re_magic_cell);
                apply(magic_name[1], split_code[1], split_code[2]);
                std::cout << std::flush;
                kernel_res["status"] = "ok";
            }

            std::regex re_magic_line(R"(^\%(\w+))");
            if (std::regex_search(code, magic_name, re_magic_line))
            {
                if (!contains(magic_name.str(1), xmagic_type::line))
                {
                    std::cerr << "Unknown magic line function %" << magic_name[1] << "\n";
                    std::cout << std::flush;
                    kernel_res["status"] = "error";
                    kernel_res["ename"] = "ename";
                    kernel_res["evalue"] = "evalue";
                    kernel_res["traceback"] = {};
                    return;
                }
                std::regex re_magic_line(R"(^\%(\w+(?:\s.*)?))");
                std::smatch split_code;
                std::regex_search(code, split_code, re_magic_line);
                apply(magic_name[1], split_code[1]);
                std::cout << std::flush;
                kernel_res["status"] = "ok";
            }
        }

        virtual xpreamble* clone() const override
        {
            return new xmagics_manager(*this);
        }

    private:

        std::map<std::string, std::shared_ptr<xmagic_cell>> m_magic_cell;
        std::map<std::string, std::shared_ptr<xmagic_line>> m_magic_line;
    };
}

#endif
