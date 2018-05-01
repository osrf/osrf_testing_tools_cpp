add_library(memory_tools SHARED
  custom_memory_functions.cpp
  implementation_monitoring_override.cpp
  initialize.cpp
  memory_tools.cpp
  memory_tools_service.cpp
  monitoring.cpp
  register_hooks.cpp
  testing_helpers.cpp
  verbosity.cpp
)
target_include_directories(memory_tools
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(APPLE)
  list(APPEND memory_tools_extra_test_env DYLD_INSERT_LIBRARIES=$<TARGET_FILE:memory_tools>)
elseif(UNIX)
  # On Linux like systems, add dl and use the normal library and LD_PRELOAD.
  target_link_libraries(memory_tools dl)
  list(APPEND memory_tools_extra_test_env LD_PRELOAD=$<TARGET_FILE:memory_tools>)
endif()

list(APPEND extra_lib_dirs $<TARGET_FILE_DIR:memory_tools>)
target_compile_definitions(memory_tools
  PRIVATE "TOOLS_CPP_MEMORY_TOOLS_BUILDING_DLL")

install(TARGETS memory_tools
  EXPORT memory_tools
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(EXPORT memory_tools
  DESTINATION share/${PROJECT_NAME}/cmake
  NAMESPACE "${PROJECT_NAME}::"
  FILE "memory_toolsExport.cmake"
)

set(memory_tools_extra_test_env "${memory_tools_extra_test_env}" PARENT_SCOPE)
