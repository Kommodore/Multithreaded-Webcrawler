cmake_minimum_required(VERSION 3.8)
project(BS_Praktikum2)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES Webbot.c socket.c msocket.h Queue.c Queue.h Webbot.h pc.c)
add_executable(BS_Praktikum2 ${SOURCE_FILES})