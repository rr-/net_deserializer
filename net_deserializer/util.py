import struct

def read_fmt(handle, fmt):
    size = struct.calcsize(fmt)
    read = handle.read(size)
    if len(read) != size:
        raise EOFError()
    return struct.unpack(fmt, read)[0]
