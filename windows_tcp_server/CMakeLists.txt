cmake_minimum_required(VERSION 3.25.1)

project(TCPWinServer
    VERSION 1.0
    DESCRIPTION "TCP Windows server"
    LANGUAGES CXX)

add_executable(tcpWinServer
    source/main.cpp
    source/tcpserver.h
    source/tcpserver.cpp)

target_compile_features(tcpWinServer PRIVATE cxx_std_17)