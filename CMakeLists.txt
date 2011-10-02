cmake_minimum_required(VERSION 2.8)
file(GLOB HEADERS "*.h")

project(libntg)
set(SRCS
    "combination.cpp")
add_library(libntg STATIC ${SRCS} ${HEADERS})

project(ntg)
set(SRCS
    "ntg.cpp")
add_executable(ntg ${SRCS} ${HEADERS})
target_link_libraries(ntg libntg)

project(ntg_jni)
find_package(JNI REQUIRED)
set(SRCS
    "combination.cpp"
    "libntgjni.cpp")
include_directories(${JNI_INCLUDE_DIRS})
add_library(libntgjni SHARED ${SRCS} ${HEADERS})
target_link_libraries(libntgjni libntg ${JNI_LIBRARIES})

project(ntg)
