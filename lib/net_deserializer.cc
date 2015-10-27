#include <iostream>
#include "binary_reader.h"
#include "error.h"
#include "net_deserializer.h"

using namespace net_deserializer;
using RecordReader = std::function<std::unique_ptr<Record>(BinaryReader &)>;

namespace
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

}

static std::unique_ptr<Record>
    read_serialized_stream_header(BinaryReader &reader)
{
    auto node = std::make_unique<Record>("SerializedStreamHeader");
    node->properties["root_id"] = std::to_string(reader.read<int32_t>());
    node->properties["header_id"] = std::to_string(reader.read<int32_t>());
    node->properties["major_version"] = std::to_string(reader.read<int32_t>());
    node->properties["minor_version"] = std::to_string(reader.read<int32_t>());
    return node;
}

static RecordReader read_stub(const RecordType record_type)
{
    return [=](BinaryReader &) -> std::unique_ptr<Record>
    {
        throw NotImplementedError(
            "Reading record type "
            + std::to_string(static_cast<int>(record_type))
            + " is not yet implemented");
    };
}

using RT = RecordType;
static std::map<RecordType, RecordReader> reader_map =
{
    {RT::SerializedStreamHeader,         read_serialized_stream_header},
    {RT::ClassWithId,                    read_stub(RT::ClassWithId)},
    {RT::SystemClassWithMembers,         read_stub(RT::SystemClassWithMembers)},
    {RT::ClassWithMembers,               read_stub(RT::ClassWithMembers)},
    {RT::SystemClassWithMembersAndTypes, read_stub(RT::SystemClassWithMembersAndTypes)},
    {RT::ClassWithMembersAndTypes,       read_stub(RT::ClassWithMembersAndTypes)},
    {RT::BinaryObjectString,             read_stub(RT::BinaryObjectString)},
    {RT::BinaryArray,                    read_stub(RT::BinaryArray)},
    {RT::MemberPrimitiveTyped,           read_stub(RT::MemberPrimitiveTyped)},
    {RT::MemberReference,                read_stub(RT::MemberReference)},
    {RT::ObjectNull,                     read_stub(RT::ObjectNull)},
    {RT::MessageEnd,                     read_stub(RT::MessageEnd)},
    {RT::BinaryLibary,                   read_stub(RT::BinaryLibary)},
    {RT::ObjectNullMultiple256,          read_stub(RT::ObjectNullMultiple256)},
    {RT::ObjectNullMultiple,             read_stub(RT::ObjectNullMultiple)},
    {RT::ArraySinglePrimitive,           read_stub(RT::ArraySinglePrimitive)},
    {RT::ArraySingleObject,              read_stub(RT::ArraySingleObject)},
    {RT::ArraySingleString,              read_stub(RT::ArraySingleString)},
    {RT::MethodCall,                     read_stub(RT::MethodCall)},
    {RT::MethodReturn,                   read_stub(RT::MethodReturn)},
};

Record::Record(const std::string &name) : name(name)
{
}

std::unique_ptr<Record>
    net_deserializer::deserialize(const std::string &input)
{
    std::vector<unsigned char> v(input.begin(), input.end());
    return net_deserializer::deserialize(v);
}

std::unique_ptr<Record>
    net_deserializer::deserialize(const std::vector<unsigned char> &input)
{
    BinaryReader input_reader(input);
    auto root = std::make_unique<Record>("Root");
    while (!input_reader.eof())
    {
        const RecordType record_type = input_reader.read<RecordType>();
        if (reader_map.find(record_type) == reader_map.end())
        {
            throw NotImplementedError("Unknown record type: "
                + std::to_string(static_cast<int>(record_type)));
        }
        auto child_record = reader_map[record_type](input_reader);
        root->children.push_back(std::move(child_record));
    }

    return root;
}
