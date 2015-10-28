#pragma once

#include "binary_reader.h"
#include "nodes.h"

namespace net_deserializer
{
    template<typename T> std::unique_ptr<LeafNode> make_primitive(
        const std::string &name, BinaryReader &reader);
}
