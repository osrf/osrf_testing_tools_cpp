# Create tests for the memory tools library.
add_executable(test_memory_tools test_memory_tools.cpp)
target_link_libraries(test_memory_tools
  memory_tools
  gtest_main
)

if(memory_tools_is_available)
  add_test(
    NAME "test_memory_tools"
    COMMAND
      "$<TARGET_FILE:test_runner>"
      --env
        ${memory_tools_extra_test_env}
      --
      "$<TARGET_FILE:test_memory_tools>"
  )
endif()
