#pragma once

#include "binary_reader.h"
#include "nodes.h"

namespace net_deserializer
{
    std::unique_ptr<Node> read_primitive(BinaryReader &reader);
}
