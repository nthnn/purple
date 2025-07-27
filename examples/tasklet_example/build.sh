mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -fcoroutines -pthread -o bin/tasklet_example examples/tasklet_example/tasklet_example.cpp src/aetherium/concurrent/*
