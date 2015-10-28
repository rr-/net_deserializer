#include "records.h"
#include "error.h"
#include "primitives.h"
#include "primitives_utils.h"

using namespace net_deserializer;

using NodeFactory = std::function<std::unique_ptr<Node>(BinaryReader &)>;

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

static std::unique_ptr<Node> read_serialized_stream_header(BinaryReader &reader)
{
    auto node = std::make_unique<AggregateNode>("SerializedStreamHeader");
    node->add(make_primitive<int32_t>("RootId", reader));
    node->add(make_primitive<int32_t>("HeaderId", reader));
    node->add(make_primitive<int32_t>("MajorVersion", reader));
    node->add(make_primitive<int32_t>("MinorVersion", reader));
    return std::move(node);
}

static std::unique_ptr<Node> read_array_single_primitive(BinaryReader &reader)
{
    throw NotImplementedError("Array of single primitive");
}

static std::unique_ptr<Node> read_array_single_object(BinaryReader &reader)
{
    auto node = std::make_unique<AggregateNode>("Array");
    auto elements_node = std::make_unique<AggregateNode>("Elements");
    node->add(make_primitive<uint32_t>("ObjectId", reader));
    node->add(make_primitive<uint32_t>("Length", reader));
    node->add(std::move(elements_node));
    return node;
}

static std::unique_ptr<Node> read_array_single_string(BinaryReader &reader)
{
    throw NotImplementedError("Array of single string");
}

static std::unique_ptr<Node> read_binary_method_call(BinaryReader &reader)
{
    auto node = std::make_unique<AggregateNode>("BinaryMethodCall");
    const auto flags = reader.read<uint32_t>();
    node->add(std::make_unique<LeafNode>("MessageEnum", flags));
    node->add(read_primitive("MethodName", reader));
    node->add(read_primitive("TypeName", reader));
    if (flags & MessageFlags::ContextInline)
        node->add(read_primitive("CallContext", reader));
    if (flags & MessageFlags::ArgsInline)
    {
        auto args_node = read_array_single_primitive(reader);
        args_node->name = "Args";
        node->add(std::move(args_node));
    }
    return std::move(node);
}

static NodeFactory read_stub(const RecordType record_type)
{
    return [=](BinaryReader &) -> std::unique_ptr<Node>
    {
        throw NotImplementedError(
            "Reading record type "
            + std::to_string(static_cast<int>(record_type))
            + " is not yet implemented");
    };
}

using RT = RecordType;
static std::map<RecordType, NodeFactory> node_factory_map =
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
    {RT::ArraySinglePrimitive,           read_array_single_primitive},
    {RT::ArraySingleObject,              read_array_single_object},
    {RT::ArraySingleString,              read_array_single_string},
    {RT::MethodCall,                     read_binary_method_call},
    {RT::MethodReturn,                   read_stub(RT::MethodReturn)},
};

std::unique_ptr<Node> net_deserializer::read_record(BinaryReader &reader)
{
    const RecordType record_type = reader.read<RecordType>();
    if (node_factory_map.find(record_type) == node_factory_map.end())
    {
        throw NotImplementedError("Unknown record type: "
            + std::to_string(static_cast<int>(record_type)));
    }
    return node_factory_map[record_type](reader);
}
