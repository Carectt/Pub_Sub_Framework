cmake_minimum_required(VERSION 3.10)
project(Pub_Sub_Framework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 如果没有指定构建类型，则默认设置为 Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "Default build type set to: ${CMAKE_BUILD_TYPE}")
endif()
# 输出目录统一设置为 build 下
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# 头文件目录
include_directories(${CMAKE_SOURCE_DIR}/Inc)

# 源文件
set(SOURCES
    Src/Broker.cpp
    Src/Msg.cpp
    Src/Pub_Sub.cpp
    Src/Topic.cpp
    Src/main.cpp
)

# 可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})
