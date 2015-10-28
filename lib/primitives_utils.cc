#include "primitives_utils.h"

namespace net_deserializer
{
    template<typename T> std::unique_ptr<Node> make_primitive(BinaryReader &r);

    template<> std::unique_ptr<Node> make_primitive<bool>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(
            static_cast<bool>(r.read<uint8_t>())); // TODO: uint32_t?
    }

    template<> std::unique_ptr<Node> make_primitive<char>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<char>());
    }

    template<> std::unique_ptr<Node> make_primitive<int8_t>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<int8_t>());
    }

    template<> std::unique_ptr<Node> make_primitive<int16_t>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<int16_t>());
    }

    template<> std::unique_ptr<Node> make_primitive<int32_t>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<int32_t>());
    }

    template<> std::unique_ptr<Node> make_primitive<int64_t>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<int64_t>());
    }

    template<> std::unique_ptr<Node> make_primitive<uint8_t>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<uint8_t>());
    }

    template<> std::unique_ptr<Node> make_primitive<uint16_t>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<uint16_t>());
    }

    template<> std::unique_ptr<Node> make_primitive<uint32_t>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<uint32_t>());
    }

    template<> std::unique_ptr<Node> make_primitive<uint64_t>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<uint64_t>());
    }

    template<> std::unique_ptr<Node> make_primitive<float>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<float>());
    }

    template<> std::unique_ptr<Node> make_primitive<double>(BinaryReader &r)
    {
        return std::make_unique<PrimitiveNode>(r.read<double>());
    }
}
