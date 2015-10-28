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
        virtual std::string as_xml(int depth = 0) const = 0;
    };

    class PrimitiveNode final : public Node
    {
    public:
        PrimitiveNode();
        PrimitiveNode(const bool value);
        PrimitiveNode(const char value);
        PrimitiveNode(const int8_t value);
        PrimitiveNode(const int16_t value);
        PrimitiveNode(const int32_t value);
        PrimitiveNode(const int64_t value);
        PrimitiveNode(const uint8_t value);
        PrimitiveNode(const uint16_t value);
        PrimitiveNode(const uint32_t value);
        PrimitiveNode(const uint64_t value);
        PrimitiveNode(const float value);
        PrimitiveNode(const double value);
        PrimitiveNode(const std::string &value);
        std::string as_xml(int depth = 0) const override;

        std::string value;
    };

    struct ObjectNode : public Node
    {
    public:
        ObjectNode(const std::string &name);

        std::string name;
    };

    struct ListNode final : public ObjectNode
    {
    public:
        ListNode(const std::string &name);
        std::string as_xml(int depth = 0) const override;

        std::vector<std::unique_ptr<Node>> elements;
    };

    struct DictionaryNode final : public ObjectNode
    {
    public:
        DictionaryNode(const std::string &name);
        std::string as_xml(int depth = 0) const override;

        std::map<std::string, std::unique_ptr<Node>> properties;
    };
}
