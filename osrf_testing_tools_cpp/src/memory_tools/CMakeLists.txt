add_library(osrf_testing_tools_cpp_memory_tools SHARED memory_tools.cpp)
target_include_directories(osrf_testing_tools_cpp_memory_tools
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(NOT WIN32)
  # otherwise the compiler will error since the allocator functions have
  # different exception specifiers
  set_target_properties(osrf_testing_tools_cpp_memory_tools
    PROPERTIES COMPILE_FLAGS "-Wno-pedantic")
endif()

if(APPLE)
  # Create an OS X specific version of the memory tools that does interposing.
  # See: http://toves.freeshell.org/interpose/
  add_library(osrf_testing_tools_cpp_memory_tools_interpose SHARED memory_tools_osx_interpose.cpp)
  target_include_directories(osrf_testing_tools_cpp_memory_tools_interpose
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
  )
  list(APPEND extra_test_libraries osrf_testing_tools_cpp_memory_tools_interpose)
  list(APPEND extra_test_env
    DYLD_INSERT_LIBRARIES=$<TARGET_FILE:osrf_testing_tools_cpp_memory_tools_interpose>)
elseif(UNIX)
  # On Linux like systems, add dl and use the normal library and DL_PRELOAD.
  list(APPEND extra_test_libraries dl)
  list(APPEND extra_test_env DL_PRELOAD=$<TARGET_FILE:osrf_testing_tools_cpp_memory_tools>)
endif()

list(APPEND extra_lib_dirs $<TARGET_FILE_DIR:osrf_testing_tools_cpp_memory_tools>)
target_link_libraries(osrf_testing_tools_cpp_memory_tools ${extra_test_libraries})
target_compile_definitions(osrf_testing_tools_cpp_memory_tools
  PRIVATE "OSRF_TESTING_TOOLS_CPP_MEMORY_TOOLS_BUILDING_DLL")
list(APPEND extra_test_libraries osrf_testing_tools_cpp_memory_tools)

set(extra_test_libraries ${extra_test_libraries} PARENT_SCOPE)
set(extra_test_env ${extra_test_env} PARENT_SCOPE)
