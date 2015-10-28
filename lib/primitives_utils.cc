#include "primitives_utils.h"

namespace net_deserializer
{
    template<typename T> std::unique_ptr<Node> make_primitive(
        BinaryReader &reader);

    template<> std::unique_ptr<Node>
        make_primitive<bool>(BinaryReader &reader)
    {
        return std::make_unique<PrimitiveNode>(
            static_cast<bool>(reader.read<uint8_t>())); // TODO: uint32_t?
    }

    template<> std::unique_ptr<Node>
        make_primitive<int32_t>(BinaryReader &reader)
    {
        return std::make_unique<PrimitiveNode>(reader.read<int32_t>());
    }

    template<> std::unique_ptr<Node>
        make_primitive<uint32_t>(BinaryReader &reader)
    {
        return std::make_unique<PrimitiveNode>(reader.read<uint32_t>());
    }
}
