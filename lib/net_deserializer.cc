#include <iostream>
#include "net_deserializer.h"
#include "error.h"
#include "primitives.h"
#include "primitives_utils.h"

using namespace net_deserializer;

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
        BinaryLibrary                  = 12,
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

    enum class BinaryArrayType : uint8_t
    {
        Single            = 0,
        Jagged            = 1,
        Rectangular       = 2,
        SingleOffset      = 3,
        JaggedOffset      = 4,
        RectangularOffset = 5,
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

    struct ClassMembersMeta final
    {
        size_t count;
        std::vector<std::string> names;
        std::vector<BinaryType> binary_types;
        std::vector<PrimitiveType> primitive_types;
    };

    struct Context final
    {
        Context(const Context &other) = delete;
        Context(BinaryReader &reader);
        BinaryReader &reader;
        std::map<int32_t, ClassMembersMeta> class_members_meta;
    };

    using NodeFactory = std::function<std::unique_ptr<Node>(Context &)>;
}

static std::unique_ptr<Node> read_one_record(Context &c);
static std::unique_ptr<Node> read_one_record(
    Context &c, const RecordType record_type);

Context::Context(BinaryReader &reader) : reader(reader)
{
}

static bool is_class(const RecordType record_type)
{
    return record_type == RecordType::ClassWithId
        || record_type == RecordType::ClassWithMembers
        || record_type == RecordType::ClassWithMembersAndTypes
        || record_type == RecordType::SystemClassWithMembers
        || record_type == RecordType::SystemClassWithMembersAndTypes;
}

static bool is_array(const RecordType record_type)
{
    return record_type == RecordType::BinaryArray
        || record_type == RecordType::ArraySinglePrimitive
        || record_type == RecordType::ArraySingleObject
        || record_type == RecordType::ArraySingleString;
}

static std::unique_ptr<Node> read_from_binary_type(
    Context &c,
    const BinaryType binary_type,
    const PrimitiveType primitive_type)
{
    if (binary_type == BinaryType::Primitive)
        return read_primitive("Value", c.reader, primitive_type);

    if (binary_type == BinaryType::String
        || binary_type == BinaryType::PrimitiveArray
        || binary_type == BinaryType::Object
        || binary_type == BinaryType::Class
        || binary_type == BinaryType::SystemClass)
    {
        return read_one_record(c);
    }

    throw NotImplementedError(
        "Unsupported binary type: " + std::to_string(
            static_cast<uint8_t>(binary_type)));
}

static void read_class_members_meta(Context &c, const int32_t object_id)
{
    ClassMembersMeta meta;
    meta.count = c.reader.read<int32_t>();
    meta.names.resize(meta.count);
    meta.binary_types.resize(meta.count);
    meta.primitive_types.resize(meta.count);

    for (std::size_t i = 0; i < meta.count; i++)
    {
        meta.names[i]
            = read_primitive("", c.reader, PrimitiveType::String)->value;
    }

    for (std::size_t i = 0; i < meta.count; i++)
        meta.binary_types[i] = c.reader.read<BinaryType>();

    for (std::size_t i = 0; i < meta.count; i++)
    {
        if (meta.binary_types[i] == BinaryType::Primitive
            || meta.binary_types[i] == BinaryType::PrimitiveArray)
        {
            meta.primitive_types[i] = c.reader.read<PrimitiveType>();
        }
        else if (meta.binary_types[i] == BinaryType::SystemClass)
        {
            read_primitive("ClassName", c.reader, PrimitiveType::String);
        }
        else if (meta.binary_types[i] == BinaryType::Class)
        {
            read_primitive("ClassName", c.reader, PrimitiveType::String);
            read_primitive("LibraryId", c.reader, PrimitiveType::Int32);
        }
    }

    c.class_members_meta[object_id] = meta;
}

static std::unique_ptr<Node> read_class_members(
    Context &c, const int32_t object_id)
{
    std::vector<std::unique_ptr<AggregateNode>> member_nodes;
    if (c.class_members_meta.find(object_id) == c.class_members_meta.end())
    {
        throw CorruptDataError(
            "Bad reference to ObjectID " + std::to_string(object_id));
    }
    const auto &meta = c.class_members_meta[object_id];
    member_nodes.resize(meta.count);
    for (std::size_t i = 0; i < meta.count; i++)
    {
        member_nodes[i] = std::make_unique<AggregateNode>("Member");
        member_nodes[i]->add(std::make_unique<LeafNode>("Name", meta.names[i]));
        member_nodes[i]->add(std::move(read_from_binary_type(
            c, meta.binary_types[i], meta.primitive_types[i])));
    }
    auto members_node = std::make_unique<AggregateNode>("Members");
    for (auto &member_node : member_nodes)
        members_node->add(std::move(member_node));
    return members_node;
}

static std::unique_ptr<Node> read_serialized_stream_header(Context &c)
{
    auto node = std::make_unique<AggregateNode>("SerializedStreamHeader");
    node->add(read_primitive("RootId", c.reader, PrimitiveType::Int32));
    node->add(read_primitive("HeaderId", c.reader, PrimitiveType::Int32));
    node->add(read_primitive("MajorVersion", c.reader, PrimitiveType::Int32));
    node->add(read_primitive("MinorVersion", c.reader, PrimitiveType::Int32));
    return node;
}

static std::unique_ptr<Node> read_class_with_id(Context &c)
{
    auto node = std::make_unique<AggregateNode>("ClassWithId");
    const auto object_id = c.reader.read<int32_t>();
    node->add(std::make_unique<LeafNode>("ObjectId", object_id));
    const auto metadata_id = c.reader.read<int32_t>();
    node->add(std::make_unique<LeafNode>("MetadataId", metadata_id));
    node->add(read_class_members(c, metadata_id));
    c.class_members_meta[object_id] = c.class_members_meta[metadata_id];
    return node;
}

static std::unique_ptr<Node> read_system_class_with_members(Context &c)
{
    throw NotImplementedError("System class with members");
}

static std::unique_ptr<Node> read_class_with_members(Context &c)
{
    throw NotImplementedError("Class with members");
}

static std::unique_ptr<Node> read_system_class_with_members_and_types(
    Context &c)
{
    auto node = std::make_unique<AggregateNode>(
        "SystemClassWithMembersAndTypes");
    const auto object_id = c.reader.read<int32_t>();
    node->add(std::make_unique<LeafNode>("ObjectId", object_id));
    node->add(read_primitive("ObjectName", c.reader, PrimitiveType::String));
    read_class_members_meta(c, object_id);
    node->add(read_class_members(c, object_id));
    return node;
}

static std::unique_ptr<Node> read_class_with_members_and_types(Context &c)
{
    auto node = std::make_unique<AggregateNode>("ClassWithMembersAndTypes");
    const auto object_id = c.reader.read<int32_t>();
    node->add(std::make_unique<LeafNode>("ObjectId", object_id));
    node->add(read_primitive("ObjectName", c.reader, PrimitiveType::String));
    read_class_members_meta(c, object_id);
    node->add(read_primitive("LibraryId", c.reader, PrimitiveType::Int32));
    node->add(read_class_members(c, object_id));
    return node;
}

static std::unique_ptr<Node> read_binary_object_string(Context &c)
{
    auto node = std::make_unique<AggregateNode>("BinaryStringObject");
    node->add(read_primitive("ObjectId", c.reader, PrimitiveType::Int32));
    node->add(read_primitive("Value", c.reader, PrimitiveType::String));
    return node;
}

static std::unique_ptr<Node> read_binary_array(Context &c)
{
    auto node = std::make_unique<AggregateNode>("BinaryArray");
    node->add(read_primitive("ObjectId", c.reader, PrimitiveType::Int32));
    const auto array_type = c.reader.read<BinaryArrayType>();
    node->add(std::make_unique<LeafNode>(
        "BinaryArrayType", static_cast<uint8_t>(array_type)));

    const std::size_t rank = c.reader.read<int32_t>();
    node->add(std::make_unique<LeafNode>("Rank", rank));

    auto dimensions_node = std::make_unique<AggregateNode>("Dimensions");
    std::vector<std::size_t> dimensions(rank);
    std::size_t element_count = 0;
    for (std::size_t i = 0; i < rank; i++)
    {
        const auto dimension = c.reader.read<int32_t>();
        dimensions.push_back(dimension);
        dimensions_node->add(std::make_unique<LeafNode>("", dimensions.back()));
        if (i == 0)
            element_count = dimension;
        else
            element_count *= dimension;
    }
    node->add(std::move(dimensions_node));

    if (array_type == BinaryArrayType::SingleOffset
        || array_type == BinaryArrayType::JaggedOffset
        || array_type == BinaryArrayType::RectangularOffset)
    {
        auto lower_bounds_node = std::make_unique<AggregateNode>("LowerBounds");
        std::vector<std::size_t> lower_bounds(rank);
        for (std::size_t i = 0; i < rank; i++)
        {
            lower_bounds.push_back(c.reader.read<int32_t>());
            lower_bounds_node->add(
                std::make_unique<LeafNode>("", lower_bounds.back()));
        }
        node->add(std::move(lower_bounds_node));
    }

    const auto binary_type = c.reader.read<BinaryType>();
    PrimitiveType primitive_type = PrimitiveType::Invalid;
    if (binary_type == BinaryType::Primitive
        || binary_type == BinaryType::PrimitiveArray)
    {
        primitive_type = c.reader.read<PrimitiveType>();
    }
    else if (binary_type == BinaryType::SystemClass)
    {
        read_primitive("ClassName", c.reader, PrimitiveType::String);
    }
    else if (binary_type == BinaryType::Class)
    {
        read_primitive("ClassName", c.reader, PrimitiveType::String);
        read_primitive("LibraryId", c.reader, PrimitiveType::Int32);
    }

    std::vector<std::unique_ptr<Node>> element_nodes;
    for (std::size_t i = 0; i < element_count; i++)
    {
        element_nodes.push_back(read_from_binary_type(
            c, binary_type, primitive_type));
    }

    auto elements_node = std::make_unique<AggregateNode>("Elements");
    for (auto &element_node : element_nodes)
        elements_node->add(std::move(element_node));
    node->add(std::move(elements_node));
    return node;
}

static std::unique_ptr<Node> read_member_primitive_typed(Context &c)
{
    throw NotImplementedError("Member primitive typed");
}

static std::unique_ptr<Node> read_member_reference(Context &c)
{
    return read_primitive("MemberReference", c.reader, PrimitiveType::Int32);
}

static std::unique_ptr<Node> read_object_null(Context &c)
{
    return std::make_unique<LeafNode>("NullObject");
}

static std::unique_ptr<Node> read_message_end(Context &c)
{
    return std::make_unique<LeafNode>("MessageEnd");
}

static std::unique_ptr<Node> read_binary_library(Context &c)
{
    auto library_node = std::make_unique<AggregateNode>("BinaryLibrary");
    library_node->add(
        read_primitive("LibraryId", c.reader, PrimitiveType::Int32));
    library_node->add(
        read_primitive("LibraryName", c.reader, PrimitiveType::String));
    const auto record_type = c.reader.read<RecordType>();
    if (is_class(record_type))
    {
        auto root_node = std::make_unique<AggregateNode>("Class");
        root_node->add(std::move(library_node));
        root_node->add(read_one_record(c, record_type));
        return root_node;
    }
    else if (is_array(record_type))
    {
        auto root_node = std::make_unique<AggregateNode>("Class");
        root_node->add(std::move(library_node));
        root_node->add(read_one_record(c, record_type));
        return root_node;
    }
    throw CorruptDataError(
        "Binary library followed with neither class or array");
}

static std::unique_ptr<Node> read_object_null_multiple_256(Context &c)
{
    throw NotImplementedError("Object null multiple 256");
}

static std::unique_ptr<Node> read_object_null_multiple(Context &c)
{
    throw NotImplementedError("Object null multiple");
}

static std::unique_ptr<Node> read_array_single_primitive(Context &c)
{
    auto node = std::make_unique<AggregateNode>("ArraySinglePrimitive");
    node->add(read_primitive("ObjectId", c.reader, PrimitiveType::Int32));
    const std::size_t length = c.reader.read<int32_t>();
    const auto primitive_type = c.reader.read<PrimitiveType>();
    auto elements_node = std::make_unique<AggregateNode>("Elements");
    for (std::size_t i = 0; i < length; i++)
        elements_node->add(read_primitive("Element", c.reader, primitive_type));
    node->add(std::move(elements_node));
    return node;
}

static std::unique_ptr<Node> read_array_single_object(Context &c)
{
    auto node = std::make_unique<AggregateNode>("ArraySingleObject");
    auto elements_node = std::make_unique<AggregateNode>("Elements");
    node->add(read_primitive("ObjectId", c.reader, PrimitiveType::Int32));
    const auto length = c.reader.read<uint32_t>();
    for (std::size_t i = 0; i < length; i++)
        elements_node->add(read_one_record(c));
    node->add(std::move(elements_node));
    return node;
}

static std::unique_ptr<Node> read_array_of_value_with_code(Context &c)
{
    throw NotImplementedError("Array of value with code");
}

static std::unique_ptr<Node> read_array_single_string(Context &c)
{
    throw NotImplementedError("Array of single string");
}

static std::unique_ptr<Node> read_method_call(Context &c)
{
    auto node = std::make_unique<AggregateNode>("BinaryMethodCall");
    const auto flags = c.reader.read<uint32_t>();
    node->add(std::make_unique<LeafNode>("Flags", flags));
    node->add(read_primitive("MethodName", c.reader));
    node->add(read_primitive("TypeName", c.reader));
    if (flags & MessageFlags::ContextInline)
        node->add(read_primitive("CallContext", c.reader));
    if (flags & MessageFlags::ArgsInline)
    {
        auto args_node = read_array_of_value_with_code(c);
        args_node->name = "Args";
        node->add(std::move(args_node));
    }
    else if (!(flags & MessageFlags::NoArgs))
    {
        auto args_node = read_one_record(c);
        args_node->name = "Args";
        node->add(std::move(args_node));
    }
    return std::move(node);
}

static std::unique_ptr<Node> read_method_return(Context &c)
{
    throw NotImplementedError("Method return");
}

using RT = RecordType;
static std::map<RecordType, NodeFactory> node_factory_map =
{
    {RT::SerializedStreamHeader,         read_serialized_stream_header},
    {RT::ClassWithId,                    read_class_with_id},
    {RT::SystemClassWithMembers,         read_system_class_with_members},
    {RT::ClassWithMembers,               read_class_with_members},
    {RT::SystemClassWithMembersAndTypes, read_system_class_with_members_and_types},
    {RT::ClassWithMembersAndTypes,       read_class_with_members_and_types},
    {RT::BinaryObjectString,             read_binary_object_string},
    {RT::BinaryArray,                    read_binary_array},
    {RT::MemberPrimitiveTyped,           read_member_primitive_typed},
    {RT::MemberReference,                read_member_reference},
    {RT::ObjectNull,                     read_object_null},
    {RT::MessageEnd,                     read_message_end},
    {RT::BinaryLibrary,                  read_binary_library},
    {RT::ObjectNullMultiple256,          read_object_null_multiple_256},
    {RT::ObjectNullMultiple,             read_object_null_multiple},
    {RT::ArraySinglePrimitive,           read_array_single_primitive},
    {RT::ArraySingleObject,              read_array_single_object},
    {RT::ArraySingleString,              read_array_single_string},
    {RT::MethodCall,                     read_method_call},
    {RT::MethodReturn,                   read_method_return},
};

static std::unique_ptr<Node> read_one_record(
    Context &c, const RecordType record_type)
{
    if (node_factory_map.find(record_type) == node_factory_map.end())
    {
        throw NotImplementedError("Unknown record type: "
            + std::to_string(static_cast<int>(record_type)));
    }
    return node_factory_map[record_type](c);
}

static std::unique_ptr<Node> read_one_record(Context &c)
{
    const RecordType record_type = c.reader.read<RecordType>();
    return read_one_record(c, record_type);
}

std::unique_ptr<Node> net_deserializer::deserialize(
    const std::vector<unsigned char> &input)
{
    BinaryReader reader(input);
    Context context(reader);
    auto root = std::make_unique<AggregateNode>("Root");
    while (!reader.eof())
    {
        try
        {
            auto child_node = read_one_record(context);
            root->elements.push_back(std::move(child_node));
        }
        catch (...)
        {
            std::cerr << root->as_xml() << std::endl;
            throw;
        }
    }
    return root;
}

std::unique_ptr<Node> net_deserializer::deserialize(const std::string &input)
{
    std::vector<unsigned char> v(input.begin(), input.end());
    return net_deserializer::deserialize(v);
}
