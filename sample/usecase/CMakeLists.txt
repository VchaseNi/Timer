cmake_minimum_required(VERSION 3.12)
project(usecase)  

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")

set(TIMER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
# include 
include_directories(
    ${TIMER_DIR}
)

# 可执行文件列表
set(EXECUTABLES once span period mixed)

foreach(bin IN LISTS EXECUTABLES)
    add_executable(${bin} 
        ${CMAKE_CURRENT_SOURCE_DIR}/${bin}.cpp
        ${TIMER_DIR}/timer.cpp  
    )
    target_compile_features(${bin} PRIVATE cxx_std_17)
    target_link_libraries(${bin} pthread)
endforeach()