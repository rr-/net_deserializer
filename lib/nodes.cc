#include <sstream>
#include "nodes.h"

using namespace net_deserializer;

static std::string pad(int depth)
{
    return std::string(depth * 4, ' ');
}

Node::Node(const std::string &name) : name(name)
{
}

LeafNode::LeafNode(const std::string &name)
    : Node(name), value("") {}
LeafNode::LeafNode(const std::string &name, const bool value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const char value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const int8_t value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const int16_t value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const int32_t value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const int64_t value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const uint8_t value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const uint16_t value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const uint32_t value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const uint64_t value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const float value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const double value)
    : Node(name), value(std::to_string(value)) {}
LeafNode::LeafNode(const std::string &name, const std::string &value)
    : Node(name), value(value) {}

std::string LeafNode::as_xml(int depth) const
{
    std::stringstream ret;
    std::string screen_name = name;
    if (screen_name.empty())
        screen_name = "Node";
    if (value.empty())
        ret << pad(depth) << "<" << screen_name << "/>\n";
    else
    {
        ret << pad(depth) << "<" << screen_name << ">"
            << value
            << "</" << screen_name << ">\n";
    }
    return ret.str();
}

AggregateNode::AggregateNode(const std::string &name) : Node(name)
{
}

std::string AggregateNode::as_xml(int depth) const
{
    std::stringstream ret;
    std::string screen_name = name;
    if (screen_name.empty())
        screen_name = "NodeList";
    if (elements.empty())
    {
        ret << pad(depth) << "<" << screen_name << "/>\n";
        return ret.str();
    }
    ret << pad(depth) << "<" << screen_name << ">\n";
    for (const auto &element : elements)
        ret << element->as_xml(depth + 1);
    ret << pad(depth) << "</" << screen_name << ">\n";
    return ret.str();
}

void AggregateNode::add(std::unique_ptr<Node> child)
{
    elements.push_back(std::move(child));
}
