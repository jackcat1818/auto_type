cmake_minimum_required(VERSION 3.14)
project(auto_type)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cpr REQUIRED)
add_executable(auto_type auto_type.cpp)
target_link_libraries(auto_type PRIVATE cpr::cpr)