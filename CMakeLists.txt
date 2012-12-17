project(c11)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

IF (CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11 -D_REENTRANT -lpthread -D_GLIBCXX_USE_NANOSLEEP")
ELSE (CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -D_REENTRANT -lpthread -D_GLIBCXX_USE_NANOSLEEP")
ENDIF (CMAKE_BUILD_TYPE MATCHES Release)



add_executable(${PROJECT_NAME} dispatcher.h worker.h worker.cpp workerlockless.h workerlockless.cpp main.cpp)
