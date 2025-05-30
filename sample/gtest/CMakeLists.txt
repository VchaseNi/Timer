cmake_minimum_required(VERSION 3.5)
project(test)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")

# include 
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# src
file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB timer ${CMAKE_CURRENT_SOURCE_DIR}/../../timer.cpp)
set(SUB_DIR_LIB_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest.cpp
    ${timer}
    ${src}
)
add_executable(${PROJECT_NAME} ${SUB_DIR_LIB_SRCS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} gtest pthread)