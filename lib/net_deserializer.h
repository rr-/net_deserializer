#pragma once

#include <map>
#include <memory>
#include <vector>

namespace net_deserializer
{
    struct Record
    {
        Record(const std::string &name);

        std::string name;
        std::map<std::string, std::string> properties;
        std::vector<std::unique_ptr<Record>> children;
    };

    std::unique_ptr<Record> deserialize(const std::string &input);

    std::unique_ptr<Record> deserialize(
        const std::vector<unsigned char> &input);
}
