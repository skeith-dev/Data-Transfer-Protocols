cmake_minimum_required(VERSION 3.20)
project(Data_Transfer_Protocols)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Data_Transfer_Protocols_client
        src/client.cpp
        src/prompts.h
        src/packet_file_io.h)

add_executable(Data_Transfer_Protocols_server
        src/server.cpp
        src/prompts.h
        src/packet_file_io.h)

add_executable(Data_Transfer_Protocols_test
        src/test.cpp
        src/prompts.h
        src/packet_file_io.h)