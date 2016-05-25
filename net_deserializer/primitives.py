from enum import Enum
from net_deserializer import util

class PrimitiveType(Enum):
    # pylint: disable=bad-whitespace
    Invalid  = 0
    Boolean  = 1
    Byte     = 2
    Char     = 3
    Decimal  = 5
    Double   = 6
    Int16    = 7
    Int32    = 8
    Int64    = 9
    SByte    = 10
    Single   = 11
    TimeSpan = 12
    DateTime = 13
    UInt16   = 14
    UInt32   = 15
    UInt64   = 16
    Null     = 17
    String   = 18

def _read_timespan(handle):
    return util.read_fmt(handle, '<q')

def _read_string(handle):
    length = 0
    for i in range(5):
        num = util.read_fmt(handle, 'B')
        length = length | (num & 0x7F) << (i * 7)
        if num & 0x80 == 0:
            break
    ret = handle.read(length)
    return ret.decode('utf-8')

_PRIMITIVE_FACTORY_MAP = {
    PrimitiveType.Boolean:  lambda h: util.read_fmt(h, '?'),
    PrimitiveType.Byte:     lambda h: util.read_fmt(h, 'B'),
    PrimitiveType.Double:   lambda h: util.read_fmt(h, 'd'),
    PrimitiveType.Int16:    lambda h: util.read_fmt(h, '<h'),
    PrimitiveType.Int32:    lambda h: util.read_fmt(h, '<l'),
    PrimitiveType.Int64:    lambda h: util.read_fmt(h, '<q'),
    PrimitiveType.SByte:    lambda h: util.read_fmt(h, 'c'),
    PrimitiveType.Single:   lambda h: util.read_fmt(h, 'f'),
    PrimitiveType.TimeSpan: _read_timespan,
    PrimitiveType.UInt16:   lambda h: util.read_fmt(h, '<H'),
    PrimitiveType.UInt32:   lambda h: util.read_fmt(h, '<L'),
    PrimitiveType.UInt64:   lambda h: util.read_fmt(h, '<Q'),
    PrimitiveType.Null:     lambda h: None,
    PrimitiveType.String:   _read_string,
}

def read_primitive(handle, primitive_type):
    try:
        return _PRIMITIVE_FACTORY_MAP[primitive_type](handle)
    except KeyError:
        raise NotImplementedError(
            'Reading primitive type {0} is not supported'.format(primitive_type))
