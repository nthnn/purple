mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -fcoroutines -pthread -o bin/cron_example examples/cron_example/cron_example.cpp src/aetherium/concurrent/* src/aetherium/cron/*
