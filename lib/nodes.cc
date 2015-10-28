#include <sstream>
#include "nodes.h"

using namespace net_deserializer;

static std::string pad(int depth)
{
    return std::string(depth * 4, ' ');
}

PrimitiveNode::PrimitiveNode(const int32_t number)
    : value(std::to_string(number))
{
}

std::string PrimitiveNode::as_xml(int depth) const
{
    return value;
}

ObjectNode::ObjectNode(const std::string &name) : name(name)
{
}

ListNode::ListNode(const std::string &name) : ObjectNode(name)
{
}

std::string ListNode::as_xml(int depth) const
{
    std::stringstream ret;
    ret << pad(depth) << "<" << name << ">\n";
    for (const auto &element : elements)
    {
        ret << element->as_xml(depth + 1);
    }
    ret << pad(depth) << "</" << name << ">\n";
    return ret.str();
}

DictionaryNode::DictionaryNode(const std::string &name) : ObjectNode(name)
{
}

std::string DictionaryNode::as_xml(int depth) const
{
    std::stringstream ret;
    ret << pad(depth) << "<" << name << ">\n";
    for (const auto &kv : properties)
    {
        bool is_primitive = dynamic_cast<PrimitiveNode*>(kv.second.get());
        ret << pad(depth + 1) << "<" << kv.first << ">";
        if (!is_primitive)
            ret << "\n";
        ret << kv.second->as_xml(depth + 2);
        if (!is_primitive)
            ret << "\n" << pad(depth + 1);
        ret << "</" << kv.first << ">\n";
    }
    ret << pad(depth) << "</" << name << ">\n";
    return ret.str();
}
