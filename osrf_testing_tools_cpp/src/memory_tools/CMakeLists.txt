add_library(memory_tools SHARED
  custom_memory_functions.cpp
  implementation_monitoring_override.cpp
  initialize.cpp
  memory_tools.cpp
  memory_tools_service.cpp
  monitoring.cpp
  register_hooks.cpp
  testing_helpers.cpp
)
target_include_directories(memory_tools
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set(libraries_to_install memory_tools)

if(APPLE)
  # # Create a macOS specific version of the memory tools that does interposing.
  # # See: http://toves.freeshell.org/interpose/
  # add_library(memory_tools_interpose_macos SHARED memory_tools_osx_interpose.cpp)
  # target_include_directories(memory_tools_interpose_macos
  #   PUBLIC
  #     $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  #     $<INSTALL_INTERFACE:include>
  # )
  # target_link_libraries(memory_tools_interpose_macos
  #   memory_tools
  # )

  # list(APPEND libraries_to_install memory_tools_interpose_macos)
  # list(APPEND extra_test_env
  #   DYLD_INSERT_LIBRARIES=$<TARGET_FILE:memory_tools_interpose_macos>)
  list(APPEND extra_test_env DYLD_INSERT_LIBRARIES=$<TARGET_FILE:memory_tools>)
elseif(UNIX)
  # On Linux like systems, add dl and use the normal library and LD_PRELOAD.
  list(APPEND extra_test_libraries dl)
  list(APPEND extra_test_env LD_PRELOAD=$<TARGET_FILE:memory_tools>)
endif()

list(APPEND extra_lib_dirs $<TARGET_FILE_DIR:memory_tools>)
target_link_libraries(memory_tools ${extra_test_libraries})
target_compile_definitions(memory_tools
  PRIVATE "TOOLS_CPP_MEMORY_TOOLS_BUILDING_DLL")
list(APPEND extra_test_libraries memory_tools)

install(
  TARGETS ${libraries_to_install}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

set(extra_test_libraries ${extra_test_libraries} PARENT_SCOPE)
set(extra_test_env ${extra_test_env} PARENT_SCOPE)
