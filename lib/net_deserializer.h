#pragma once

#include <memory>
#include <vector>

namespace net_deserializer
{
    enum class RecordType : uint8_t
    {
        SerializedStreamHeader         = 0,
        ClassWithId                    = 1,
        SystemClassWithMembers         = 2,
        ClassWithMembers               = 3,
        SystemClassWithMembersAndTypes = 4,
        ClassWithMembersAndTypes       = 5,
        BinaryObjectString             = 6,
        BinaryArray                    = 7,
        MemberPrimitiveTyped           = 8,
        MemberReference                = 9,
        ObjectNull                     = 10,
        MessageEnd                     = 11,
        BinaryLibary                   = 12,
        ObjectNullMultiple256          = 13,
        ObjectNullMultiple             = 14,
        ArraySinglePrimitive           = 15,
        ArraySingleObject              = 16,
        ArraySingleString              = 17,
        MethodCall                     = 21,
        MethodReturn                   = 22,
    };

    enum class BinaryType : uint8_t
    {
        Primitive      = 0,
        String         = 1,
        Object         = 2,
        SystemClass    = 3,
        Class          = 4,
        ObjectArray    = 5,
        StringArray    = 6,
        PrimitiveArray = 7,
    };

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

    enum MessageFlags
    {
        NoArgs                 = 0x00000001,
        ArgsInline             = 0x00000002,
        ArgsIsArray            = 0x00000004,
        ArgsInArray            = 0x00000008,
        NoContext              = 0x00000010,
        ContextInline          = 0x00000020,
        ContextInArray         = 0x00000040,
        MethodSignatureInArray = 0x00000080,
        PropertiesInArray      = 0x00000100,
        NoReturnValue          = 0x00000200,
        ReturnValueVoid        = 0x00000400,
        ReturnValueInline      = 0x00000800,
        ReturnValueInArray     = 0x00001000,
        ExceptionInArray       = 0x00002000,
        GenericMethod          = 0x00008000,
    };

    struct Record
    {
        uint8_t record_type;
        std::vector<std::unique_ptr<Record>> children;
        std::vector<unsigned char> binary_data;
    };

    struct RootRecord : public Record
    {
        int32_t root_id;
        int32_t header_id;
        int32_t major_version;
        int32_t minor_version;
    };

    std::unique_ptr<Record> deserialize(
        const std::string &input);

    std::unique_ptr<Record> deserialize(
        const std::vector<unsigned char> &input);
}
