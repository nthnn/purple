mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -o bin/card_vld_example examples/card_vld_example/card_vld_example.cpp src/netlet/helper/*
