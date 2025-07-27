#include <aetherium/helper/uuid.hpp>
#include <iostream>

int main() {
    using namespace Aetherium::Helper;

    UUIDGenerator generator;
    std::cout << "Generating 5 UUIDs (Version 4):"
        << std::endl;

    for(int i = 0; i < 5; ++i) {
        std::string uuid = generator.generate();
        std::cout << "UUID "
            << (i + 1)
            << ": "
            << uuid
            << std::endl;
    }

    return 0;
}
