#!/bin/bash
g++ -o bin/tmake src/arguments.cpp src/builder.cpp src/executor.cpp src/lexer.cpp src/main.cpp src/parser.cpp -std=c++17 -Wall -Wextra -O2
