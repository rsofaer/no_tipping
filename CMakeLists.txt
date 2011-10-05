cmake_minimum_required(VERSION 2.8)
file(GLOB HEADERS "*.h")

find_package(OpenMP REQUIRED)

project(ntg)
set(SRCS
    "combination.cpp")
add_library(ntg STATIC ${SRCS} ${HEADERS})

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif(UNIX)

project(no_tipping_game)
set(SRCS
    "ntg.cpp")
add_executable(no_tipping_game ${SRCS} ${HEADERS})
target_link_libraries(no_tipping_game ntg)
if(WIN32)
  set_target_properties(no_tipping_game PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
endif(WIN32)

project(ntgjni)
find_package(JNI REQUIRED)
set(SRCS
    "libntgjni.cpp")
include_directories(${JNI_INCLUDE_DIRS})
add_library(ntgjni SHARED ${SRCS} ${HEADERS})
target_link_libraries(ntgjni ntg ${JNI_LIBRARIES})
if(WIN32)
  set_target_properties(ntgjni PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
endif(WIN32)

if(HPS_GTEST_ENABLED)
  project(ntg_gtest)
  set(SRCS
      "ntg_gtest.cpp")
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(ntg_gtest ${SRCS} ${HEADERS})
  target_link_libraries(ntg_gtest ntg gtest)
  if(WIN32)
    set_target_properties(ntg_gtest PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
  endif(WIN32)
endif(HPS_GTEST_ENABLED)

file(GLOB JAVA_SRCS "*.java")
file(COPY ${JAVA_SRCS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

project(ntg)
