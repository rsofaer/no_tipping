cmake_minimum_required(VERSION 2.8)

file(GLOB HEADERS "*.h")

# Enable OpenMP in UNIX.
# Enable multicore builds in MSVC.
find_package(OpenMP REQUIRED)
if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif(UNIX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  message("Setting CMAKE_INSTALL_PREFIX to ${CMAKE_BINARY_DIR}.")
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
else(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  message("CMAKE_INSTALL_PREFIX was set to ${CMAKE_INSTALL_PREFIX}.")
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

project(ntg)
set(SRCS
    "combination.cpp"
    "adversarial_utils.cpp"
    "ntg.cpp")
add_library(ntg STATIC ${SRCS} ${HEADERS})

project(ntg_contestant_util)
set(SRCS
    "contestant_util.cpp")
add_library(contestant_util STATIC ${SRCS} ${HEADERS})
target_link_libraries(contestant_util ntg)

project(ntg_contestant)
set(SRCS
    "contestant.cpp")
add_executable(contestant ${SRCS} ${HEADERS})
target_link_libraries(contestant contestant_util)
install(TARGETS contestant RUNTIME DESTINATION bin/no_tipping)
if(WIN32)
  add_custom_command(TARGET contestant
                     POST_BUILD
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                     COMMAND del /F /Q *.class
                     COMMAND javac *.java)
else(WIN32)
  add_custom_command(TARGET contestant
                     POST_BUILD
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                     COMMAND rm -f *.class
                     COMMAND javac *.java)
endif(WIN32)
install(
  CODE "file(GLOB JAVA_CLASSES \"${CMAKE_CURRENT_BINARY_DIR}/*.class\")"
  CODE "file(INSTALL \${JAVA_CLASSES} DESTINATION \"${CMAKE_INSTALL_PREFIX}/bin/no_tipping\")"
  )

if(HPS_GTEST_ENABLED)
  project(ntg_gtest)
  set(SRCS
      "ntg_gtest.cpp")
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(ntg_gtest ${SRCS} ${HEADERS})
  target_link_libraries(ntg_gtest ntg gtest)
  add_test(ntg_gtest ntg_gtest)

  project(ntg_contestant_gtest)
  set(SRCS
      "contestant_gtest.cpp")
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(contestant_gtest ${SRCS} ${HEADERS})
  target_link_libraries(contestant_gtest contestant_util gtest)
  add_test(contestant_gtest contestant_gtest)
endif(HPS_GTEST_ENABLED)

file(GLOB JAVA_SRCS "*.java")
file(COPY ${JAVA_SRCS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

project(ntg)
