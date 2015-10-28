#include "primitives.h"
#include "error.h"
#include "primitives_utils.h"

using namespace net_deserializer;

std::unique_ptr<LeafNode> net_deserializer::read_primitive(
    const std::string &name, BinaryReader &reader)
{
    const auto primitive_type = reader.read<PrimitiveType>();
    return net_deserializer::read_primitive(name, reader, primitive_type);
}

std::unique_ptr<LeafNode> net_deserializer::read_primitive(
    const std::string &name,
    BinaryReader &reader,
    const PrimitiveType primitive_type)
{
    if (primitive_type == PrimitiveType::Boolean)
        return make_primitive<bool>(name, reader);

    if (primitive_type == PrimitiveType::Byte)
        return make_primitive<uint8_t>(name, reader);

    if (primitive_type == PrimitiveType::Char)
        throw NotImplementedError("Reading Char is not implemented (UTF8)");

    if (primitive_type == PrimitiveType::Decimal)
        throw NotImplementedError("Reading Decimal is not implemented");

    if (primitive_type == PrimitiveType::Double)
        return make_primitive<double>(name, reader);

    if (primitive_type == PrimitiveType::Int16)
        return make_primitive<int16_t>(name, reader);

    if (primitive_type == PrimitiveType::Int32)
        return make_primitive<int32_t>(name, reader);

    if (primitive_type == PrimitiveType::Int64)
        return make_primitive<int64_t>(name, reader);

    if (primitive_type == PrimitiveType::UInt16)
        return make_primitive<uint16_t>(name, reader);

    if (primitive_type == PrimitiveType::UInt32)
        return make_primitive<uint32_t>(name, reader);

    if (primitive_type == PrimitiveType::UInt64)
        return make_primitive<uint64_t>(name, reader);

    if (primitive_type == PrimitiveType::SByte)
        return make_primitive<int8_t>(name, reader);

    if (primitive_type == PrimitiveType::Single)
        return make_primitive<float>(name, reader);

    if (primitive_type == PrimitiveType::TimeSpan)
        return make_primitive<int64_t>(name, reader);

    if (primitive_type == PrimitiveType::DateTime)
        return make_primitive<int64_t>(name, reader);

    if (primitive_type == PrimitiveType::Null)
        return std::make_unique<LeafNode>(name);

    if (primitive_type == PrimitiveType::String)
    {
        std::size_t length = 0;
        for (std::size_t i = 0; i < 5; i++)
        {
            const auto c = reader.read<uint8_t>();
            length |= (c & 0x7F) << (i * 7);
            if (!(c & 0x80))
                break;
        }
        const auto buf = std::make_unique<char[]>(length);
        for (std::size_t i = 0; i < length; i++)
            buf[i] = reader.read<char>();
        return std::make_unique<LeafNode>(name, std::string(buf.get(), length));
    }

    throw NotImplementedError("Unknown primitive type: "
        + std::to_string(static_cast<int>(primitive_type)));
}
