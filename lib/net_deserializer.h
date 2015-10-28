#pragma once

#include "nodes.h"

namespace net_deserializer
{
    std::unique_ptr<Node> deserialize(const std::string &input);
    std::unique_ptr<Node> deserialize(const std::vector<unsigned char> &input);
}
