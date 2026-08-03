cmake_minimum_required(VERSION 3.10)
project(Pub_Sub_Framework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 头文件目录
include_directories(${CMAKE_SOURCE_DIR}/Inc)

# 源文件
set(SOURCES
    Src/Broker.cpp
    Src/Msg.cpp
    Src/Pub_Sub.cpp
    Src/Topic.cpp
)

# 可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})
