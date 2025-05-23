cmake_minimum_required(VERSION 3.5)
project(test)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")

# include 
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# src
file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB timer ${CMAKE_CURRENT_SOURCE_DIR}/../../taskMgr.cpp)
set(SUB_DIR_LIB_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest.cpp
    ${timer}
    ${src}
)
message(${timer})
add_executable(${PROJECT_NAME} ${SUB_DIR_LIB_SRCS})
target_link_libraries(${PROJECT_NAME} gtest pthread)