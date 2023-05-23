cmake_minimum_required(VERSION 3.26.3)

project(UDPWinClient
    VERSION 1.0
    DESCRIPTION "UDP Windows client"
    LANGUAGES CXX)

add_executable(udpWinClient
    source/main.cpp
    source/udpclient.cpp
    source/udpclient.h)

target_compile_features(udpWinClient PRIVATE cxx_std_17)