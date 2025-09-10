mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -o bin/email_example examples/email_example/email_example.cpp src/netlet/helper/*
