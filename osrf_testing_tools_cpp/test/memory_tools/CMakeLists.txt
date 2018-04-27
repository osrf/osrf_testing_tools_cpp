# Create tests for the memory tools library.
add_executable(test_memory_tools test_memory_tools.cpp)
target_link_libraries(test_memory_tools
  memory_tools
  ${extra_test_libraries}
  gtest_main
)

if(NOT WIN32)  # (memory tools isn't implemented on Windows)
  add_test(
    NAME "test_memory_tools"
    COMMAND
      "$<TARGET_FILE:test_runner>"
      --env
        ${extra_test_env}
      --
      "$<TARGET_FILE:test_memory_tools>"
  )
endif()
