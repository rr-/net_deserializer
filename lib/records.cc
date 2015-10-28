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

    struct MemberReadContext
    {
        std::vector<std::unique_ptr<AggregateNode>> member_nodes;
        std::vector<BinaryType> binary_types;
        std::vector<PrimitiveType> primitive_types;
        std::vector<std::unique_ptr<Node>> additional_info;
    };
}

static MemberReadContext prepare_member_read_context(BinaryReader &reader)
{
    MemberReadContext context;
    const std::size_t member_count = reader.read<int32_t>();
    context.binary_types.resize(member_count);
    context.primitive_types.resize(member_count);
    context.additional_info.resize(member_count);

    for (std::size_t i = 0; i < member_count; i++)
    {
        context.member_nodes.push_back(
            std::make_unique<AggregateNode>("Member"));
    }

    for (std::size_t i = 0; i < member_count; i ++)
    {
        context.member_nodes[i]->add(
            read_primitive("MemberName", reader, PrimitiveType::String));
    }

    for (std::size_t i = 0; i < member_count; i++)
        context.binary_types[i] = reader.read<BinaryType>();

    for (std::size_t i = 0; i < member_count; i++)
    {
        if (context.binary_types[i] == BinaryType::Primitive
            || context.binary_types[i] == BinaryType::PrimitiveArray)
        {
            context.primitive_types[i] = reader.read<PrimitiveType>();
        }
        else if (context.binary_types[i] == BinaryType::SystemClass)
        {
            read_primitive("ClassName", reader, PrimitiveType::String);
        }
        else if (context.binary_types[i] == BinaryType::Class)
        {
            read_primitive("ClassName", reader, PrimitiveType::String);
            read_primitive("LibraryId", reader, PrimitiveType::Int32);
        }
    }
    return context;
}

static std::unique_ptr<Node> read_class_members_with_types(
    BinaryReader &reader, MemberReadContext &context)
{
    for (std::size_t i = 0; i < context.member_nodes.size(); i++)
    {
        if (context.binary_types[i] == BinaryType::Primitive)
        {
            context.member_nodes[i]->add(
                read_primitive("Value", reader, context.primitive_types[i]));
        }
        else if (context.binary_types[i] == BinaryType::String
            || context.binary_types[i] == BinaryType::Object
            || context.binary_types[i] == BinaryType::SystemClass)
        {
            context.member_nodes[i]->add(read_record(reader));
        }
        else
        {
            throw NotImplementedError(
                "Unsupported binary type: " + std::to_string(
                    static_cast<uint8_t>(context.binary_types[i])));
        }
    }

    auto members_node = std::make_unique<AggregateNode>("Members");
    for (auto &member_node : context.member_nodes)
        members_node->add(std::move(member_node));
    return members_node;
}

static std::unique_ptr<Node> read_serialized_stream_header(BinaryReader &reader)
{
    auto node = std::make_unique<AggregateNode>("SerializedStreamHeader");
    node->add(read_primitive("RootId", reader, PrimitiveType::Int32));
    node->add(read_primitive("HeaderId", reader, PrimitiveType::Int32));
    node->add(read_primitive("MajorVersion", reader, PrimitiveType::Int32));
    node->add(read_primitive("MinorVersion", reader, PrimitiveType::Int32));
    return node;
}

static std::unique_ptr<Node> read_system_class_with_members(
    BinaryReader &reader)
{
    throw NotImplementedError("System class with members");
}

static std::unique_ptr<Node> read_class_with_members(
    BinaryReader &reader)
{
    throw NotImplementedError("Class with members");
}

static std::unique_ptr<Node> read_system_class_with_members_and_types(
    BinaryReader &reader)
{
    auto node = std::make_unique<AggregateNode>("SystemObject");
    node->add(read_primitive("ObjectId", reader, PrimitiveType::Int32));
    node->add(read_primitive("ObjectName", reader, PrimitiveType::String));
    auto member_read_context = prepare_member_read_context(reader);
    // node->add(read_primitive("LibraryId", reader, PrimitiveType::Int32));
    node->add(read_class_members_with_types(reader, member_read_context));
    return node;
}

static std::unique_ptr<Node> read_class_with_members_and_types(
    BinaryReader &reader)
{
    auto node = std::make_unique<AggregateNode>("Object");
    node->add(read_primitive("ObjectId", reader, PrimitiveType::Int32));
    node->add(read_primitive("ObjectName", reader, PrimitiveType::String));
    auto member_read_context = prepare_member_read_context(reader);
    node->add(read_primitive("LibraryId", reader, PrimitiveType::Int32));
    node->add(read_class_members_with_types(reader, member_read_context));
    return node;
}

static std::unique_ptr<Node> read_binary_object_string(BinaryReader &reader)
{
    auto node = std::make_unique<AggregateNode>("StringObject");
    node->add(read_primitive("ObjectId", reader, PrimitiveType::Int32));
    node->add(read_primitive("Value", reader, PrimitiveType::String));
    return node;
}

static std::unique_ptr<Node> read_member_reference(BinaryReader &reader)
{
    return read_primitive("ObjectReference", reader, PrimitiveType::Int32);
}

static std::unique_ptr<Node> read_object_null(BinaryReader &reader)
{
    return std::make_unique<LeafNode>("NullObject");
}

static std::unique_ptr<Node> read_message_end(BinaryReader &reader)
{
    return std::make_unique<LeafNode>("MessageEnd");
}

static std::unique_ptr<Node> read_binary_library(BinaryReader &reader)
{
    auto node = std::make_unique<AggregateNode>("BinaryLibrary");
    node->add(read_primitive("LibraryId", reader, PrimitiveType::Int32));
    node->add(read_primitive("LibraryName", reader, PrimitiveType::String));
    return node;
}

static std::unique_ptr<Node> read_array_single_primitive(BinaryReader &reader)
{
    throw NotImplementedError("Array of single primitive");
}

static std::unique_ptr<Node> read_array_single_object(BinaryReader &reader)
{
    auto node = std::make_unique<AggregateNode>("Array");
    auto elements_node = std::make_unique<AggregateNode>("Elements");
    node->add(read_primitive("ObjectId", reader, PrimitiveType::Int32));
    const auto length = reader.read<uint32_t>();
    for (std::size_t i = 0; i < length; i++)
        elements_node->add(read_record(reader));
    node->add(std::move(elements_node));
    return node;
}

static std::unique_ptr<Node> read_array_of_value_with_code(BinaryReader &reader)
{
    throw NotImplementedError("Array of value with code");
}

static std::unique_ptr<Node> read_array_single_string(BinaryReader &reader)
{
    throw NotImplementedError("Array of single string");
}

static std::unique_ptr<Node> read_binary_method_call(BinaryReader &reader)
{
    auto node = std::make_unique<AggregateNode>("BinaryMethodCall");
    const auto flags = reader.read<uint32_t>();
    node->add(std::make_unique<LeafNode>("Flags", flags));
    node->add(read_primitive("MethodName", reader));
    node->add(read_primitive("TypeName", reader));
    if (flags & MessageFlags::ContextInline)
        node->add(read_primitive("CallContext", reader));
    if (flags & MessageFlags::ArgsInline)
    {
        auto args_node = read_array_of_value_with_code(reader);
        args_node->name = "Args";
        node->add(std::move(args_node));
    }
    else if (!(flags & MessageFlags::NoArgs))
    {
        auto args_node = read_record(reader);
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
    {RT::SystemClassWithMembers,         read_system_class_with_members},
    {RT::ClassWithMembers,               read_class_with_members},
    {RT::SystemClassWithMembersAndTypes, read_system_class_with_members_and_types},
    {RT::ClassWithMembersAndTypes,       read_class_with_members_and_types},
    {RT::BinaryObjectString,             read_binary_object_string},
    {RT::BinaryArray,                    read_stub(RT::BinaryArray)},
    {RT::MemberPrimitiveTyped,           read_stub(RT::MemberPrimitiveTyped)},
    {RT::MemberReference,                read_member_reference},
    {RT::ObjectNull,                     read_object_null},
    {RT::MessageEnd,                     read_message_end},
    {RT::BinaryLibary,                   read_binary_library},
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
