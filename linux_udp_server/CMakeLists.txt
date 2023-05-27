cmake_minimum_required(VERSION 3.26.3)

project(UDPLinuxServer
    VERSION 1.0
    DESCRIPTION "UPD Linux server"
    LANGUAGES CXX)

add_executable(udpLinServer
    source/main.cpp
    source/udpserver.h
    source/udpserver.cpp)

target_compile_features(udpLinServer PRIVATE cxx_std_17)