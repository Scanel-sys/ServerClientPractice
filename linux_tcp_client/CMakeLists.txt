cmake_minimum_required(VERSION 3.25.1)

project(TCPLinuxClient
    VERSION 1.0
    DESCRIPTION "TCP Linux client"
    LANGUAGES CXX)

add_executable(tcpLinClient
    source/main.cpp
    source/tcpclient.h
    source/tcpclient.cpp)

target_compile_features(tcpLinClient PRIVATE cxx_std_17)