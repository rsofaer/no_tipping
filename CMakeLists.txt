cmake_minimum_required(VERSION 2.8)

project(no_tipping_game)

file(GLOB HEADERS "*.h")

set(SRCS
    "main.cpp")

add_executable(ntg ${SRCS} ${HEADERS})
