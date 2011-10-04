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

project(ntgjni)
find_package(JNI REQUIRED)
set(SRCS
    "libntgjni.cpp")
include_directories(${JNI_INCLUDE_DIRS})
add_library(ntgjni SHARED ${SRCS} ${HEADERS})
target_link_libraries(ntgjni libntg ${JNI_LIBRARIES})
SET_TARGET_PROPERTIES(ntgjni PROPERTIES COMPILE_FLAGS -fPIC)
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fPIC")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fPIC")

if(HPS_GTEST_ENABLED)
  project(ntg_gtest)
  set(SRCS
      "ntg_gtest.cpp")
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(ntg_gtest ${SRCS} ${HEADERS})
  target_link_libraries(ntg_gtest libntg gtest)
endif(HPS_GTEST_ENABLED)

project(ntg)
