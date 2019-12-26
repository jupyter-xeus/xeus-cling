/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XCPP_INSPECT_HPP
#define XCPP_INSPECT_HPP

#include <fstream>
#include <string>

#include <dirent.h>

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Value.h"
#include "cling/MetaProcessor/MetaProcessor.h"
#include "cling/Utils/Output.h"

#include "pugixml.hpp"

#include "xeus-cling/xbuffer.hpp"
#include "xeus-cling/xpreamble.hpp"

#include "xdemangle.hpp"
#include "xparser.hpp"
#include "xpaths.hpp"

namespace xcpp
{
    struct node_predicate
    {
        std::string kind;
        std::string child_value;

        bool operator()(pugi::xml_node node) const
        {
            return static_cast<std::string>(node.attribute("kind").value()) == kind && static_cast<std::string>(node.child("name").child_value()) == child_value;
        }
    };

    struct class_member_predicate
    {
        std::string class_name;
        std::string kind;
        std::string child_value;

        std::string get_filename(pugi::xml_node node)
        {
            for (pugi::xml_node child : node.children())
            {
                if (static_cast<std::string>(child.attribute("kind").value()) == kind &&
                    static_cast<std::string>(child.child("name").child_value()) == child_value)
                {
                    return child.child("anchorfile").child_value();
                }
            }
            return "";
        }

        bool operator()(pugi::xml_node node)
        {
            auto parent = (static_cast<std::string>(node.attribute("kind").value()) == "class" ||
                           static_cast<std::string>(node.attribute("kind").value()) == "struct") &&
                static_cast<std::string>(node.child("name").child_value()) == class_name;
            auto found = false;
            if (parent)
            {
                for (pugi::xml_node child : node.children())
                {
                    if (static_cast<std::string>(child.attribute("kind").value()) == kind &&
                        static_cast<std::string>(child.child("name").child_value()) == child_value)
                    {
                        found = true;
                        break;
                    }
                }
            }
            return found;
        }
    };

    std::string find_type(const std::string& expression, cling::MetaProcessor& m_processor)
    {
        cling::Interpreter::CompilationResult compilation_result;
        cling::Value result;
        std::string typeString;

        // add typeinfo in include files in order to use typeid
        std::string code = "#include <typeinfo>";
        m_processor.process(code.c_str(), compilation_result, &result);

        // try to find the typename of the class
        code = "typeid(" + expression + ").name();";

        // Temporarily dismissing all std::cerr and std::cout resulting from `m_processor.process`
        auto errorlevel = 0;
        {
            auto cout_strbuf = std::cout.rdbuf();
            auto cerr_strbuf = std::cerr.rdbuf();
            auto null = xnull();
            std::cout.rdbuf(&null);
            std::cerr.rdbuf(&null);

            errorlevel = m_processor.process(code.c_str(), compilation_result, &result);

            std::cout.rdbuf(cout_strbuf);
            std::cerr.rdbuf(cerr_strbuf);
        }

        if (errorlevel)
        {
            m_processor.cancelContinuation();
        }
        else if (compilation_result == cling::Interpreter::kSuccess)
        {
            // we found the typeid
            std::string valueString;
            {
                llvm::raw_string_ostream os(valueString);
                result.print(os);
            }

            // search the typename in the output between ""
            std::regex re_typename("\\\"(.*)\\\"");
            std::smatch typename_;
            std::regex_search(valueString, typename_, re_typename);
            // set in valueString the typename given by typeid
            valueString = typename_.str(1);
            // we need demangling in order to have its string representation
            valueString = demangle(valueString);

            re_typename = "(\\w*(?:\\:{2}?\\w*)*)";
            std::regex_search(valueString, typename_, re_typename);
            if (!typename_.str(1).empty())
            {
                typeString = typename_[1];
            }
        }

        return typeString;
    }

    static nl::json read_tagconfs(const char* path)
    {
        nl::json result = nl::json::array();
        DIR* directory = opendir(path);
        if (directory == nullptr)
        {
            return result;
        }
        dirent* item = readdir(directory);
        while (item != nullptr)
        {
            std::string extension = "json";
            if (item->d_type == DT_REG)
            {
                std::string fname = item->d_name;

                if (fname.find(extension, (fname.length() - extension.length())) != std::string::npos)
                {
                    std::ifstream i(path + ('/' + fname));
                    nl::json entry;
                    i >> entry;
                    result.emplace_back(std::move(entry));
                }
            }
            item = readdir(directory);
        }
        closedir(directory);
        return result;
    }

    void inspect(const std::string& code, nl::json& kernel_res, cling::MetaProcessor& m_processor)
    {
        std::string tagconf_dir = prefix_path() + XCPP_TAGCONFS_DIR;
        std::string tagfiles_dir = prefix_path() + XCPP_TAGFILES_DIR;

        nl::json tagconfs = read_tagconfs(tagconf_dir.c_str());

        std::vector<std::string> check{"class", "struct", "function"};

        std::string url, tagfile;

        std::regex re_expression(R"((((?:\w*(?:\:{2}|\<.*\>|\(.*\)|\[.*\])?)\.?)*))");
        std::smatch inspect;
        std::regex_search(code, inspect, re_expression);

        std::string inspect_result;

        std::smatch method;
        std::string to_inspect = inspect[1];

        // Method or variable of class found (xxxx.yyyy)
        if (std::regex_search(to_inspect, method, std::regex(R"((.*)\.(\w*)$)")))
        {
            std::string typename_ = find_type(method[1], m_processor);

            if (!typename_.empty())
            {
                for (nl::json::const_iterator it = tagconfs.cbegin(); it != tagconfs.cend(); ++it)
                {
                    url = it->at("url");
                    tagfile = it->at("tagfile");
                    std::string filename = tagfiles_dir + "/" + tagfile;
                    pugi::xml_document doc;
                    pugi::xml_parse_result result = doc.load_file(filename.c_str());
                    class_member_predicate predicate{typename_, "function", method[2]};
                    auto node = doc.find_node(predicate);
                    if (!node.empty())
                    {
                        inspect_result = url + predicate.get_filename(node);
                    }
                }
            }
        }
        else
        {
            std::string find_string;

            // check if we try to find the documentation of a namespace
            // if yes, don't try to find the type using typeid
            std::regex is_namespace(R"(\w+(\:{2}\w+)+)");
            std::smatch namespace_match;
            if (std::regex_match(to_inspect, namespace_match, is_namespace))
            {
                find_string = to_inspect;
            }
            else
            {
                std::string typename_ = find_type(to_inspect, m_processor);
                find_string = (typename_.empty()) ? to_inspect : typename_;
            }

            for (nl::json::const_iterator it = tagconfs.cbegin(); it != tagconfs.cend(); ++it)
            {
                url = it->at("url");
                tagfile = it->at("tagfile");
                std::string filename = tagfiles_dir + "/" + tagfile;
                pugi::xml_document doc;
                pugi::xml_parse_result result = doc.load_file(filename.c_str());
                for (auto c : check)
                {
                    node_predicate predicate{c, find_string};
                    std::string node;

                    if (c == "class" || c == "struct")
                    {
                        node = doc.find_node(predicate).child("filename").child_value();
                    }
                    else
                    {
                        node = doc.find_node(predicate).child("anchorfile").child_value();
                    }

                    if (!node.empty())
                    {
                        inspect_result = url + node;
                    }
                }
            }
        }

        if (inspect_result.empty())
        {
            std::cerr << "No documentation found for " << code << "\n";
            std::cout << std::flush;
            kernel_res["found"] = false;
            kernel_res["status"] = "error";
            kernel_res["ename"] = "No documentation found";
            kernel_res["evalue"] = "";
            kernel_res["traceback"] = nl::json::array();
        }
        else
        {
            // Format html content.
            std::string html_content = R"(<style>
            #pager-container {
                padding: 0;
                margin: 0;
                width: 100%;
                height: 100%;
            }
            .xcpp-iframe-pager {
                padding: 0;
                margin: 0;
                width: 100%;
                height: 100%;
                border: none;
            }
            </style>
            <iframe class="xcpp-iframe-pager" src=")" +
                inspect_result + R"(?action=purge"></iframe>)";

            // Note: Adding "?action=purge" suffix to force cppreference's
            // Mediawiki to purge the HTTP cache.

            kernel_res["payload"] = nl::json::array();
            kernel_res["payload"][0] = nl::json::object({
                {"data", {
                    {"text/plain", inspect_result},
                    {"text/html", html_content}}
                },
                {"source", "page"},
                {"start", 0}
            });
            kernel_res["user_expressions"] = nl::json::object();

            std::cout << std::flush;
            kernel_res["found"] = true;
            kernel_res["status"] = "ok";
        }
    }

    class xintrospection : public xpreamble
    {
    public:

        using xpreamble::pattern;
        const std::string spattern = R"(^\?)";

        xintrospection(cling::MetaProcessor& p)
            : m_processor{p}
        {
            pattern = spattern;
        }

        void apply(const std::string& code, nl::json& kernel_res) override
        {
            std::regex re(spattern + R"((.*))");
            std::smatch to_inspect;
            std::regex_search(code, to_inspect, re);
            inspect(to_inspect[1], kernel_res, m_processor);
        }

        virtual xpreamble* clone() const override
        {
            return new xintrospection(*this);
        }

    private:

        cling::MetaProcessor& m_processor;
    };
}
#endif
