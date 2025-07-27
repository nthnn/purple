mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -o bin/uuid_example examples/uuid_example/uuid_example.cpp src/aetherium/helper/*
