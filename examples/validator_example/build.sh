mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -o bin/validator_example examples/validator_example/validator_example.cpp src/netlet/helper/*
