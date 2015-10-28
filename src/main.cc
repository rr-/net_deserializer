#include <fstream>
#include <iostream>
#include "net_deserializer.h"

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        std::cerr << "No file was given.";
        return 1;
    }

    std::ifstream input_file(std::string(argv[1]), std::ios::binary);
    std::string buffer(
        (std::istreambuf_iterator<char>(input_file)),
        (std::istreambuf_iterator<char>()));

    try
    {
        auto root_node = net_deserializer::deserialize(buffer);
        std::cerr << "Finished" << std::endl;
        std::cout << root_node->as_xml();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
