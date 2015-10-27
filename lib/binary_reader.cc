#include "binary_reader.h"
#include "error.h"

using namespace net_deserializer;

BinaryReader::BinaryReader(const std::vector<unsigned char> &input)
    : source(input.begin(), input.end()),
    source_ptr(&source[0]),
    source_end(&source[source.size()])
{
}

BinaryReader::~BinaryReader()
{
}

bool BinaryReader::eof() const
{
    return source_ptr >= source_end;
}

void BinaryReader::skip(const std::size_t n)
{
    if (source_ptr + n > source_end)
        throw CorruptDataError("Premature end of file");
    source_ptr += n;
}
