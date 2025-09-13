mkdir -p bin
g++ -Wall -Weffc++ -std=c++20 -Iinclude -o bin/weblet_employee.so -fPIC -shared examples/weblet_example/weblet_employee.cpp src/purple/cron/* src/purple/concurrent/* src/purple/net/*
g++ -Wall -Weffc++ -std=c++20 -Iinclude -o bin/weblet_example examples/weblet_example/weblet_example.cpp src/purple/cron/* src/purple/concurrent/* src/purple/net/*
