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

project(libntgjni)
find_package(JNI REQUIRED)
set(SRCS
    "libntgjni.cpp")
include_directories(${JNI_INCLUDE_DIRS})
add_library(libntgjni SHARED ${SRCS} ${HEADERS})
target_link_libraries(libntgjni libntg ${JNI_LIBRARIES})
SET_TARGET_PROPERTIES(libntgjni PROPERTIES COMPILE_FLAGS -fPIC)

if(HPS_GTEST_ENABLED)
  project(ntg_gtest)
  set(SRCS
      "ntg_gtest.cpp")
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(ntg_gtest ${SRCS} ${HEADERS})
  target_link_libraries(ntg_gtest libntg gtest)
endif(HPS_GTEST_ENABLED)

project(ntg)
