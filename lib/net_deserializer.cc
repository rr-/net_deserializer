#include "net_deserializer.h"
#include "records.h"

using namespace net_deserializer;

std::unique_ptr<Node> net_deserializer::deserialize(const std::string &input)
{
    std::vector<unsigned char> v(input.begin(), input.end());
    return net_deserializer::deserialize(v);
}

std::unique_ptr<Node> net_deserializer::deserialize(
    const std::vector<unsigned char> &input)
{
    BinaryReader input_reader(input);
    auto root = std::make_unique<ListNode>("Root");
    while (!input_reader.eof())
    {
        auto child_node = read_record(input_reader);
        root->elements.push_back(std::move(child_node));
    }
    return root;
}
