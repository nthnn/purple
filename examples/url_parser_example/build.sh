mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -o bin/url_parser_example examples/url_parser_example/url_parser_example.cpp src/purple/helper/*
