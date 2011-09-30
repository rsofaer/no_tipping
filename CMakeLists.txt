cmake_minimum_required(VERSION 2.8)

project(no_tipping_game)

file(GLOB HEADERS "*.h")

set(SRCS
    "brute_force_ntg.cpp")

add_executable(brute_force_ntg ${SRCS} ${HEADERS})

