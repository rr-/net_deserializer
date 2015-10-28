#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace net_deserializer
{
    class Node
    {
    public:
        Node(const std::string &name);
        virtual std::string as_xml(int depth = 0) const = 0;

        std::string name;
    };

    class LeafNode final : public Node
    {
    public:
        LeafNode(const std::string &name);
        LeafNode(const std::string &name, const bool value);
        LeafNode(const std::string &name, const char value);
        LeafNode(const std::string &name, const int8_t value);
        LeafNode(const std::string &name, const int16_t value);
        LeafNode(const std::string &name, const int32_t value);
        LeafNode(const std::string &name, const int64_t value);
        LeafNode(const std::string &name, const uint8_t value);
        LeafNode(const std::string &name, const uint16_t value);
        LeafNode(const std::string &name, const uint32_t value);
        LeafNode(const std::string &name, const uint64_t value);
        LeafNode(const std::string &name, const float value);
        LeafNode(const std::string &name, const double value);
        LeafNode(const std::string &name, const std::string &value);
        std::string as_xml(int depth = 0) const override;

        std::string value;
    };

    struct AggregateNode final : public Node
    {
    public:
        AggregateNode(const std::string &name);
        void add(std::unique_ptr<Node> child);
        std::string as_xml(int depth = 0) const override;

        std::vector<std::unique_ptr<Node>> elements;
    };
}
