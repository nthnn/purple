mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -o bin/env_example examples/env_example/env_example.cpp src/aetherium/format/*
