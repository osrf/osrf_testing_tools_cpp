cmake_minimum_required(VERSION 3.5)

project(test_osrf_testing_tools_cpp)

# Default to C++11
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

find_package(osrf_testing_tools_cpp REQUIRED)

if(BUILD_TESTING)
  include(CTest)
  include(cmake/osrf_testing_tools_cpp_require_googletest.cmake)
  osrf_testing_tools_cpp_require_googletest(VERSION_GTE 1.7)  # ensures target gtest_main exists

  add_subdirectory(test)
endif()
