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

    std::string inspect(const std::string& code, int cursor_pos)
    {
        std::string delims = " \t\n`!@#$^&*()=+[{]}\\|;\'\",<>?";
        std::size_t _cursor_pos = cursor_pos;
        auto text = split_line(code, delims, _cursor_pos);
        std::string to_inspect = text.back().c_str();
        std::string url = "http://en.cppreference.com/w/";
        std::string tagfile_path = TAGFILE_DIR;

        std::vector<std::string> check{"class", "struct", "function"};
        for(auto c: check)
        {
            node_predicate predicate{c, to_inspect};
            pugi::xml_document doc;
            std::string filename = tagfile_path + "/cppreference-doxygen-web.tag.xml";
            pugi::xml_parse_result result = doc.load_file(filename.c_str());
            std::string node;
            if (c == "class" || c == "struct")
                node = doc.find_node(predicate).child("filename").child_value();
            else
                node = doc.find_node(predicate).child("anchorfile").child_value();
            if (!node.empty())
                return url + node;
        }
        return "";
    }
}
#endif