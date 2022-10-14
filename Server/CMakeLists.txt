cmake_minimum_required(VERSION 3.16)
project(kaspersky_server)
set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.71 REQUIRED COMPONENTS log_setup log thread)
add_executable(server main.cpp)

target_link_libraries(server ${Boost_LIBRARIES})