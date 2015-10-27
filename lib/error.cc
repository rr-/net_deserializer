#include "error.h"

using namespace net_deserializer;

NotImplementedError::NotImplementedError(const std::string &msg)
    : std::runtime_error(msg)
{
}

CorruptDataError::CorruptDataError(const std::string &msg)
    : std::runtime_error(msg)
{
}
