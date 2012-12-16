project(c11)
#set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11 -D_REENTRANT -lpthread -D_GLIBCXX_USE_NANOSLEEP")
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -D_REENTRANT -lpthread -D_GLIBCXX_USE_NANOSLEEP")


cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})
