cmake_minimum_required(VERSION 3.0)
project(server)
include_directories(../ser)
set(SOURCES
    ../ser/test.cpp
)
add_executeable(test $(SOURCES))