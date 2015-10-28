#include "primitives_utils.h"

namespace net_deserializer
{
    template<typename T> std::unique_ptr<LeafNode>
        make_primitive(BinaryReader &r);

    template<> std::unique_ptr<LeafNode> make_primitive<bool>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(
            name,
            static_cast<bool>(r.read<uint8_t>())); // TODO: uint32_t?
    }

    template<> std::unique_ptr<LeafNode> make_primitive<char>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<char>());
    }

    template<> std::unique_ptr<LeafNode> make_primitive<int8_t>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<int8_t>());
    }

    template<> std::unique_ptr<LeafNode> make_primitive<int16_t>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<int16_t>());
    }

    template<> std::unique_ptr<LeafNode> make_primitive<int32_t>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<int32_t>());
    }

    template<> std::unique_ptr<LeafNode> make_primitive<int64_t>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<int64_t>());
    }

    template<> std::unique_ptr<LeafNode> make_primitive<uint8_t>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<uint8_t>());
    }

    template<> std::unique_ptr<LeafNode> make_primitive<uint16_t>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<uint16_t>());
    }

    template<> std::unique_ptr<LeafNode> make_primitive<uint32_t>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<uint32_t>());
    }

    template<> std::unique_ptr<LeafNode> make_primitive<uint64_t>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<uint64_t>());
    }

    template<> std::unique_ptr<LeafNode> make_primitive<float>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<float>());
    }

    template<> std::unique_ptr<LeafNode> make_primitive<double>(
        const std::string &name, BinaryReader &r)
    {
        return std::make_unique<LeafNode>(name, r.read<double>());
    }
}
