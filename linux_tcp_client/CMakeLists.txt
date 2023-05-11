cmake_minimum_required(VERSION 3.26.3)

project(TCPLinuxClient
    VERSION 1.0
    DESCRIPTION "TCP Linux client"
    LANGUAGES CXX)

add_executable(tcpLinuxClient
    source/main.cpp
    source/tcpclient.h
    source/tcpclient.cpp)

target_compile_features(tcpLinuxClient PRIVATE cxx_std_17)