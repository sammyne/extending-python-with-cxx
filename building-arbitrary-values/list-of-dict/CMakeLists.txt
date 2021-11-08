cmake_minimum_required(VERSION 3.16)

project(list-of-tuple)

set(CMAKE_CXX_STANDARD 20)

include_directories(/usr/local/include/python3.10)
link_libraries(python3.10)

add_executable(app main.cc)
