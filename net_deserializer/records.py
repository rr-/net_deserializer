from enum import Enum
from net_deserializer import util
from net_deserializer.node import Node, LeafNode
from net_deserializer.primitives import read_primitive, PrimitiveType

class CorruptDataError(RuntimeError):
    pass

class RecordType(Enum):
    # pylint: disable=bad-whitespace
    SerializedStreamHeader         = 0
    ClassWithId                    = 1
    SystemClassWithMembers         = 2
    ClassWithMembers               = 3
    SystemClassWithMembersAndTypes = 4
    ClassWithMembersAndTypes       = 5
    BinaryObjectString             = 6
    BinaryArray                    = 7
    MemberPrimitiveTyped           = 8
    MemberReference                = 9
    ObjectNull                     = 10
    MessageEnd                     = 11
    BinaryLibrary                  = 12
    ObjectNullMultiple256          = 13
    ObjectNullMultiple             = 14
    ArraySinglePrimitive           = 15
    ArraySingleObject              = 16
    ArraySingleString              = 17
    MethodCall                     = 21
    MethodReturn                   = 22

class BinaryType(Enum):
    # pylint: disable=bad-whitespace
    Primitive      = 0
    String         = 1
    Object         = 2
    SystemClass    = 3
    Class          = 4
    ObjectArray    = 5
    StringArray    = 6
    PrimitiveArray = 7

class BinaryArrayType(Enum):
    # pylint: disable=bad-whitespace
    Single            = 0
    Jagged            = 1
    Rectangular       = 2
    SingleOffset      = 3
    JaggedOffset      = 4
    RectangularOffset = 5

class MessageFlags(Enum):
    # pylint: disable=bad-whitespace
    NoArgs                 = 0x00000001
    ArgsInline             = 0x00000002
    ArgsIsArray            = 0x00000004
    ArgsInArray            = 0x00000008
    NoContext              = 0x00000010
    ContextInline          = 0x00000020
    ContextInArray         = 0x00000040
    MethodSignatureInArray = 0x00000080
    PropertiesInArray      = 0x00000100
    NoReturnValue          = 0x00000200
    ReturnValueVoid        = 0x00000400
    ReturnValueInline      = 0x00000800
    ReturnValueInArray     = 0x00001000
    ExceptionInArray       = 0x00002000
    GenericMethod          = 0x00008000

class Context():
    def __init__(self, handle):
        self.handle = handle
        self.objects = {}

def _is_class(record_type):
    return record_type in (
        RecordType.ClassWithId,
        RecordType.ClassWithMembers,
        RecordType.ClassWithMembersAndTypes,
        RecordType.SystemClassWithMembers,
        RecordType.SystemClassWithMembersAndTypes)

def _is_array(record_type):
    return record_type in(
        RecordType.BinaryArray,
        RecordType.ArraySinglePrimitive,
        RecordType.ArraySingleObject,
        RecordType.ArraySingleString)

def _read_create_object(ctx, name):
    object_id = read_primitive(ctx.handle, PrimitiveType.Int32)
    node = Node(name)
    ctx.objects[object_id] = node
    node.add_leaf('ObjectId', object_id)
    return node, object_id

def _read_binary_type(ctx):
    return BinaryType(util.read_fmt(ctx.handle, 'B'))

def _read_record_type(ctx):
    return RecordType(util.read_fmt(ctx.handle, 'B'))

def _read_primitive_type(ctx):
    return PrimitiveType(util.read_fmt(ctx.handle, 'B'))

def _read_from_binary_type(ctx, binary_type, primitive_type):
    if binary_type == BinaryType.Primitive:
        return LeafNode('Value', read_primitive(ctx.handle, primitive_type))

    if binary_type in (
            BinaryType.String,
            BinaryType.Object,
            BinaryType.PrimitiveArray,
            BinaryType.StringArray,
            BinaryType.ObjectArray,
            BinaryType.Class,
            BinaryType.SystemClass):
        record_type = _read_record_type(ctx)
        return _read_record(ctx, record_type)

    raise NotImplementedError(
        'Unimplemented binary type: {0}'.format(binary_type))

def _read_class_members_meta(ctx, object_id):
    count = read_primitive(ctx.handle, PrimitiveType.Int32)
    names = [None] * count
    binary_types = [None] * count
    for i in range(count):
        names[i] = read_primitive(ctx.handle, PrimitiveType.String)
    for i in range(count):
        binary_types[i] = _read_binary_type(ctx)

    node = Node('MembersMeta')
    for i in range(count):
        member_node = Node('MemberMeta')
        member_node.add_leaf('Name', names[i])
        member_node.add_leaf('BinaryType', binary_types[i])
        if binary_types[i] in (BinaryType.Primitive, BinaryType.PrimitiveArray):
            member_node.add_leaf('PrimitiveType', _read_primitive_type(ctx))
        elif binary_types[i] == BinaryType.SystemClass:
            member_node.add_leaf(
                'ClassName', read_primitive(ctx.handle, PrimitiveType.String))
        elif binary_types[i] == BinaryType.Class:
            member_node.add_leaf(
                'ClassName', read_primitive(ctx.handle, PrimitiveType.String))
            member_node.add_leaf(
                'LibraryId', read_primitive(ctx.handle, PrimitiveType.Int32))
        node.add(member_node)
    return node

def _read_class_members(ctx, object_id):
    if not object_id in ctx.objects:
        raise CorruptDataError(
            'Bad reference to ObjectID {0}'.format(object_id))
    members_meta = ctx.objects[object_id].get('MembersMeta')
    member_nodes = Node('Members')
    for i, member_meta in enumerate(members_meta.children):
        member_node = Node('Member')
        member_node.add(member_meta.get('Name'))
        member_node.add(_read_from_binary_type(
            ctx,
            member_meta.get_leaf('BinaryType'),
            member_meta.get_leaf('PrimitiveType', None)))
        member_nodes.add(member_node)
    return member_nodes

def _read_serialized_stream_header(ctx):
    node = Node('SerializedStreamHeader')
    node.add_leaf('RootId', read_primitive(ctx.handle, PrimitiveType.Int32))
    node.add_leaf('HeaderId', read_primitive(ctx.handle, PrimitiveType.Int32))
    node.add_leaf('MajorVersion', read_primitive(ctx.handle, PrimitiveType.Int32))
    node.add_leaf('MinorVersion', read_primitive(ctx.handle, PrimitiveType.Int32))
    return node

def _read_class_with_id(ctx):
    node, object_id = _read_create_object(ctx, 'ClassWithId')
    metadata_id = util.read_fmt(ctx.handle, '<l')
    node.add(ctx.objects[metadata_id].get('MembersMeta'))
    node.add(_read_class_members(ctx, metadata_id))
    return node

def _read_system_class_with_members_and_types(ctx):
    node, object_id = _read_create_object(ctx, 'SystemClassWithMembersAndTypes')
    node.add_leaf('ObjectName', read_primitive(ctx.handle, PrimitiveType.String))
    node.add(_read_class_members_meta(ctx, object_id))
    node.add(_read_class_members(ctx, object_id))
    return node

def _read_class_with_members_and_types(ctx):
    node, object_id = _read_create_object(ctx, 'ClassWithMembersAndTypes')
    node.add_leaf('ObjectName', read_primitive(ctx.handle, PrimitiveType.String))
    node.add(_read_class_members_meta(ctx, object_id))
    node.add_leaf('LibraryId', read_primitive(ctx.handle, PrimitiveType.Int32))
    node.add(_read_class_members(ctx, object_id))
    return node

def _read_binary_object_string(ctx):
    node, object_id = _read_create_object(ctx, 'BinaryObjectString')
    node.add_leaf('Value', read_primitive(ctx.handle, PrimitiveType.String))
    return node

def _read_binary_array(ctx):
    node, object_id = _read_create_object(ctx, 'BinaryArray')
    array_type = BinaryArrayType(util.read_fmt(ctx.handle, 'B'))
    node.add_leaf('BinaryArrayType', array_type)

    rank = util.read_fmt(ctx.handle, '<l')
    node.add_leaf('Rank', rank)

    dimensions_node = Node('Dimensions')
    element_count = 0
    for i in range(rank):
        dimension = util.read_fmt(ctx.handle, '<l')
        dimensions_node.add_leaf('Dimension', dimension)
        if i == 0:
            element_count = dimension
        else:
            element_count *= dimension
    node.add(dimensions_node)

    if array_type in (
            BinaryArrayType.SingleOffset,
            BinaryArrayType.JaggedOffset,
            BinaryArrayType.RectangularOffset):
        lower_bounds_node = Node('LowerBounds')
        for i in range(rank):
            lower_bounds_node.add_leaf(
                'LowerBound', util.read_fmt(ctx.handle, '<l'))
        node.add(lower_bounds_node)

    binary_type = _read_binary_type(ctx)
    primitive_type = PrimitiveType.Invalid
    if binary_type in (BinaryType.Primitive, BinaryType.PrimitiveArray):
        primitive_type = _read_primitive_type(ctx)
    elif binary_type == BinaryType.SystemClass:
        node.add_leaf('ClassName', read_primitive(ctx.handle, PrimitiveType.String))
    elif binary_type == BinaryType.Class:
        node.add_leaf('ClassName', read_primitive(ctx.handle, PrimitiveType.String))
        node.add_leaf('LibraryId', read_primitive(ctx.handle, PrimitiveType.Int32))

    elements_node = Node('Elements')
    for i in range(element_count):
        elements_node.add(_read_from_binary_type(ctx, binary_type, primitive_type))
    node.add(elements_node)
    return node

def _read_member_primitive_typed(ctx):
    primitive_type = _read_primitive_type(ctx)
    return LeafNode(
        'MemberPrimitiveTyped',
        read_primitive(ctx.handle, primitive_type))

def _read_member_reference(ctx):
    return LeafNode(
        'MemberReference',
        read_primitive(ctx.handle, PrimitiveType.Int32))

def _read_object_null(ctx):
    return LeafNode('NullObject')

def _read_message_end(ctx):
    return LeafNode('MessageEnd')

def _read_binary_library(ctx):
    library_node = Node('BinaryLibrary')
    library_node.add_leaf('LibraryId', read_primitive(ctx.handle, PrimitiveType.Int32))
    library_node.add_leaf('LibraryName', read_primitive(ctx.handle, PrimitiveType.String))
    record_type = _read_record_type(ctx)
    if _is_class(record_type):
        library_node.add(_read_record(ctx, record_type))
        return library_node
    elif _is_array(record_type):
        library_node.add(_read_record(ctx, record_type))
        return library_node
    else:
        raise CorruptDataError(
            'Binary library followed with neither class or array')

def _read_array_single_primitive(ctx):
    node, object_id = _read_create_object(ctx, 'ArraySinglePrimitive')
    length = util.read_fmt(ctx.handle, '<l')
    primitive_type = _read_primitive_type(ctx)
    elements_node = Node('Elements')
    for i in range(length):
        elements_node.add_leaf('Element', read_primitive(ctx.handle, primitive_type))
    node.add(elements_node)
    return node

def _read_array_single_string(ctx):
    node, object_id = _read_create_object(ctx, 'ArraySingleString')
    length = util.read_fmt(ctx.handle, '<L')
    elements_node = Node('Elements')
    for i in range(length):
        record_type = _read_record_type(ctx)
        elements_node.add_leaf('Element', _read_record(ctx, record_type))
    node.add(elements_node)
    return node

def _read_array_single_object(ctx):
    node, object_id = _read_create_object(ctx, 'ArraySingleObject')
    length = util.read_fmt(ctx.handle, '<l')
    record_type = _read_record_type(ctx)
    elements_node = Node('Elements')
    for i in range(length):
        elements_node.add_leaf('Element', _read_record(ctx, record_type))
    node.add(elements_node)
    return node

def _read_array_of_value_with_code(ctx):
    raise NotImplementedError('Unimplemented: array of value with code')

def _read_method_call(ctx):
    node = Node('BinaryMethodCall')

    flags = util.read_fmt(ctx.handle, '<L')
    node.add_leaf('Flags', flags)

    primitive_type = _read_primitive_type(ctx)
    node.add_leaf('MethodName', read_primitive(ctx.handle, primitive_type))

    primitive_type = _read_primitive_type(ctx)
    node.add_leaf('TypeName', read_primitive(ctx.handle, primitive_type))

    if (flags & MessageFlags.ContextInline.value) != 0:
        primitive_type = _read_primitive_type(ctx)
        node.add_leaf('CallContext', read_primitive(ctx.handle, primitive_type))

    if (flags & MessageFlags.ArgsInline.value) != 0:
        args_node = _read_array_of_value_with_code(ctx)
        args_node.name = 'Args'
        node.add(args_node)
    elif (flags & MessageFlags.NoArgs.value) == 0:
        record_type = _read_record_type(ctx)
        args_node = _read_record(ctx, record_type)
        args_node.name = 'Args'
        node.add(args_node)

    return node

def _read_record(ctx, record_type):
    try:
        return _NODE_FACTORY_MAP[record_type](ctx)
    except KeyError:
        raise NotImplementedError(
            'Unimplemented record type: {0}'.format(record_type))

_NODE_FACTORY_MAP = {
    RecordType.SerializedStreamHeader:         _read_serialized_stream_header,
    RecordType.ClassWithId:                    _read_class_with_id,
    RecordType.SystemClassWithMembersAndTypes: _read_system_class_with_members_and_types,
    RecordType.ClassWithMembersAndTypes:       _read_class_with_members_and_types,
    RecordType.BinaryObjectString:             _read_binary_object_string,
    RecordType.BinaryArray:                    _read_binary_array,
    RecordType.MemberPrimitiveTyped:           _read_member_primitive_typed,
    RecordType.MemberReference:                _read_member_reference,
    RecordType.ObjectNull:                     _read_object_null,
    RecordType.MessageEnd:                     _read_message_end,
    RecordType.BinaryLibrary:                  _read_binary_library,
    RecordType.ArraySinglePrimitive:           _read_array_single_primitive,
    RecordType.ArraySingleString:              _read_array_single_string,
    RecordType.ArraySingleObject:              _read_array_single_object,
    RecordType.MethodCall:                     _read_method_call,
}

def deserialize(handle):
    ctx = Context(handle)
    root = Node('Root')
    while True:
        try:
            record_type = _read_record_type(ctx)
        except EOFError:
            break
        child = _read_record(ctx, record_type)
        root.add(child)
    return root
