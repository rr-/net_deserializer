#pragma once

#include <stdexcept>

namespace net_deserializer
{
    struct NotImplementedError : public std::runtime_error
    {
    public:
        NotImplementedError(const std::string &msg);
    };

    struct CorruptDataError : public std::runtime_error
    {
    public:
        CorruptDataError(const std::string &msg);
    };
}
