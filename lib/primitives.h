#pragma once

#include "binary_reader.h"
#include "nodes.h"

namespace net_deserializer
{
    enum class PrimitiveType : uint8_t
    {
        Invalid  = 0,
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

    std::unique_ptr<LeafNode> read_primitive(
        const std::string &name,
        BinaryReader &reader);

    std::unique_ptr<LeafNode> read_primitive(
        const std::string &name,
        BinaryReader &reader,
        const PrimitiveType primitive_type);
}
