mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -o bin/json_example examples/json_example/json_example.cpp src/netlet/format/*
