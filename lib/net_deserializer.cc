#include <iostream>
#include "binary_reader.h"
#include "error.h"
#include "net_deserializer.h"

using namespace net_deserializer;

std::unique_ptr<Record> net_deserializer::deserialize(const std::string &input)
{
    std::vector<unsigned char> v(input.begin(), input.end());
    return net_deserializer::deserialize(v);
}

std::unique_ptr<RootRecord> read_root_node(BinaryReader &reader)
{
    std::unique_ptr<RootRecord> node(new RootRecord);
    node->record_type = reader.read<uint8_t>();
    node->root_id = reader.read<int32_t>();
    node->header_id = reader.read<int32_t>();
    node->major_version = reader.read<int32_t>();
    node->minor_version = reader.read<int32_t>();
    return node;
}

std::unique_ptr<Record>
    net_deserializer::deserialize(const std::vector<unsigned char> &input)
{
    BinaryReader reader(input);
    auto node = read_root_node(reader);
    std::cerr << "record type:   " << node->record_type << std::endl;
    std::cerr << "root id:       " << node->root_id << std::endl;
    std::cerr << "header id:     " << node->header_id << std::endl;
    std::cerr << "major version: " << node->major_version << std::endl;
    std::cerr << "minor version: " << node->minor_version << std::endl;
    if (node->record_type != 0)
        throw CorruptDataError("Expected record type 0");
    if (node->root_id == 0)
        throw NotImplementedError("BinaryMethodCalls are not implemented");

    return std::move(node);
}
