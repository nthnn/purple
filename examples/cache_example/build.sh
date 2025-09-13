mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -o bin/cache_example examples/cache_example/cache_example.cpp src/purple/sys/*
