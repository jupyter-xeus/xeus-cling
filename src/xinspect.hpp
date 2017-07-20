/***************************************************************************
* Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/
#ifndef XINSPECT_HPP
#define XINSPECT_HPP

#include <pugixml.hpp>
#include <string>
#include <fstream>

#include "xparser.hpp"

namespace xeus
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
        std::string member_file;

        std::string get_filename(pugi::xml_node node)
        {
            for (pugi::xml_node child: node.children())
            {
                if (static_cast<std::string>(child.attribute("kind").value()) == kind && static_cast<std::string>(child.child("name").child_value()) == child_value)
                    return child.child("anchorfile").child_value();
            }
        }

        bool operator()(pugi::xml_node node)
        {
            auto parent = (static_cast<std::string>(node.attribute("kind").value()) == "class" || static_cast<std::string>(node.attribute("kind").value()) == "struct") && static_cast<std::string>(node.child("name").child_value()) == class_name;
            auto found = false;
            if (parent)
                for (pugi::xml_node child: node.children())
                {
                    if (static_cast<std::string>(child.attribute("kind").value()) == kind && static_cast<std::string>(child.child("name").child_value()) == child_value)
                    {
                        found = true;
                        break;
                    }
                }
            return found;
        }
    };

    std::string inspect(const std::string& code)
    {

        std::string tagfile_dir = TAGFILE_DIR;

        std::vector<std::string> check{"class", "struct", "function"};

        std::string search_file = tagfile_dir + "/search_list.txt";
        std::ifstream search(search_file);

        std::string url, tagfile;

        // method found
        if (std::regex_search(code, std::regex{"\\.\\w*\\?\\?"}))
        {
            std::regex re_expression("((((?:\\w*(?:(?:\\:{2})|(?:\\<(?:.*)\\>)|(?:\\(.*\\))|(?:\\[.*\\]))?))\\.?)*)\\?\\?");
            std::smatch to_inspect;
            std::regex_search(code, to_inspect, re_expression);
            std::cout << "to_inspect " << to_inspect[1] << "\n";
        }
        else
        {
            std::regex re_expression("((((?:\\w*(?:(?:\\:{2})|(?:\\<(?:.*)\\>)|(?:\\(.*\\))|(?:\\[.*\\]))?)))*)\\?\\?");
            std::smatch to_inspect;
            std::regex_search(code, to_inspect, re_expression);
            std::cout << "to_inspect " << to_inspect[1] << "\n";
            while(search >> url >> tagfile)
            {
                std::string filename = tagfile_dir + "/" + tagfile;
                pugi::xml_document doc;
                pugi::xml_parse_result result = doc.load_file(filename.c_str());
                for(auto c: check)
                {
                    node_predicate predicate{c, to_inspect[1]};
                    std::string node;
                    if (c == "class" || c == "struct")
                        node = doc.find_node(predicate).child("filename").child_value();
                    else
                        node = doc.find_node(predicate).child("anchorfile").child_value();
                    if (!node.empty())
                        return url + node;
                }
            }
        }
        return "";
    }
}
#endif