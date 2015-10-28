#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include "error.h"

namespace net_deserializer
{
    class BinaryReader
    {
        public:
            BinaryReader(const std::vector<unsigned char> &input);
            ~BinaryReader();
            void skip(const std::size_t n);
            bool eof() const;
            template<typename T> T read()
            {
                T ret;
                if (source_ptr + sizeof(T) > source_end)
                    throw CorruptDataError("Premature end of file");
                std::memcpy(&ret, source_ptr, sizeof(T));
                source_ptr += sizeof(T);
                return ret;
            }

        private:
            std::vector<unsigned char> source;
            unsigned char *source_ptr;
            unsigned char *source_end;
    };
}
