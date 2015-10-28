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
    {
        return make_primitive<bool>(reader);
    }
    else
    {
        throw NotImplementedError("Unknown primitive type: "
            + std::to_string(static_cast<int>(primitive_type)));
    }
}
