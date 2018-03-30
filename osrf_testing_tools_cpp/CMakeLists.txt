cmake_minimum_required(VERSION 3.5)

project(osrf_testing_tools_cpp)

# Default to C++11
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_subdirectory(src)

if(BUILD_TESTING)
  include(CTest)
  include(cmake/osrf_testing_tools_cpp_require_googletest.cmake)
  osrf_testing_tools_cpp_require_googletest(VERSION_GTE 1.7)  # ensures target gtest_main exists

  add_subdirectory(test)
endif()

# ament_add_gtest(test_memory_tools test_memory_tools.cpp
#   ENV ${extra_test_env}
#   ${SKIP_TEST}
# )
# if(TARGET test_memory_tools)
#   target_link_libraries(test_memory_tools ${extra_test_libraries})
# endif()

# set(extra_test_libraries ${extra_test_libraries} PARENT_SCOPE)
# set(extra_test_env ${extra_test_env} PARENT_SCOPE)
# set(extra_lib_dirs ${extra_lib_dirs} PARENT_SCOPE)
