cmake_minimum_required(VERSION 3.31)
project(TGcleansing)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(td)
include_directories(/home/ivanoaof/CLionProjects/TGcleansing/build)
add_executable(TGcleansing main.cpp)
target_link_libraries(TGcleansing PRIVATE Td::TdStatic)
