#include "primitives.h"
#include "error.h"
#include "primitives_utils.h"

using namespace net_deserializer;

namespace
{
    enum class PrimitiveType : uint8_t
    {
        Boolean  = 1,
        Byte     = 2,
        Char     = 3,
        Decimal  = 5,
        Double   = 6,
        Int16    = 7,
        Int32    = 8,
        Int64    = 9,
        SByte    = 10,
        Single   = 11,
        TimeSpan = 12,
        DateTime = 13,
        UInt16   = 14,
        UInt32   = 15,
        UInt64   = 16,
        Null     = 17,
        String   = 18,
    };
}

std::unique_ptr<Node> net_deserializer::read_primitive(BinaryReader &reader)
{
    const auto primitive_type = reader.read<PrimitiveType>();

    if (primitive_type == PrimitiveType::Boolean)
        return make_primitive<bool>(reader);

    if (primitive_type == PrimitiveType::Byte)
        return make_primitive<uint8_t>(reader);

    if (primitive_type == PrimitiveType::Char)
        return make_primitive<char>(reader);

    if (primitive_type == PrimitiveType::Decimal)
        return make_primitive<float>(reader); // TODO: verify

    if (primitive_type == PrimitiveType::Double)
        return make_primitive<double>(reader);

    if (primitive_type == PrimitiveType::Int16)
        return make_primitive<int16_t>(reader);

    if (primitive_type == PrimitiveType::Int32)
        return make_primitive<int32_t>(reader);

    if (primitive_type == PrimitiveType::Int64)
        return make_primitive<int64_t>(reader);

    if (primitive_type == PrimitiveType::UInt16)
        return make_primitive<uint16_t>(reader);

    if (primitive_type == PrimitiveType::UInt32)
        return make_primitive<uint32_t>(reader);

    if (primitive_type == PrimitiveType::UInt64)
        return make_primitive<uint64_t>(reader);

    if (primitive_type == PrimitiveType::SByte)
        return make_primitive<int8_t>(reader);

    if (primitive_type == PrimitiveType::Single)
        return make_primitive<float>(reader);

    if (primitive_type == PrimitiveType::TimeSpan)
        return make_primitive<uint64_t>(reader); // TODO: verify

    if (primitive_type == PrimitiveType::DateTime)
        return make_primitive<uint64_t>(reader); // TODO: verify

    if (primitive_type == PrimitiveType::Null)
        return std::make_unique<PrimitiveNode>();

    if (primitive_type == PrimitiveType::String)
    {
        std::size_t length = 0;
        for (std::size_t i = 0; i < 5; i++)
        {
            const auto c = reader.read<uint8_t>();
            length <<= 7;
            length |= (c & 0x7F);
            if (!(c & 0x80))
                break;
        }
        const auto buf = std::make_unique<char[]>(length);
        for (std::size_t i = 0; i < length; i++)
            buf[i] = reader.read<char>();
        return std::make_unique<PrimitiveNode>(std::string(buf.get(), length));
    }

    throw NotImplementedError("Unknown primitive type: "
        + std::to_string(static_cast<int>(primitive_type)));
}
