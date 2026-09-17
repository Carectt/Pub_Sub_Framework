cmake_minimum_required(VERSION 3.10)

project(PubSubProbe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(pub_sub_probe
    pub_sub_probe.cpp
    ../Src/Broker.cpp
    ../Src/Msg.cpp
    ../Src/Pub_Sub.cpp
)

target_include_directories(pub_sub_probe PRIVATE ../Inc)
target_link_libraries(pub_sub_probe PRIVATE Threads::Threads)

add_executable(pub_sub_smoke
    pub_sub_smoke.cpp
    ../Src/Broker.cpp
    ../Src/Msg.cpp
    ../Src/Pub_Sub.cpp
)
target_include_directories(pub_sub_smoke PRIVATE ../Inc)
target_link_libraries(pub_sub_smoke PRIVATE Threads::Threads)

if(MSVC)
    target_compile_options(pub_sub_probe PRIVATE /W4 /permissive-)
    target_compile_options(pub_sub_smoke PRIVATE /W4 /permissive-)
else()
    target_compile_options(pub_sub_probe PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(pub_sub_smoke PRIVATE -Wall -Wextra -Wpedantic)
endif()
