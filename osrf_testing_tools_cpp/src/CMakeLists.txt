add_subdirectory(memory_tools)
add_subdirectory(test_runner)

# Forward environment variables up from memory tools.
set(extra_test_libraries ${extra_test_libraries} PARENT_SCOPE)
set(extra_test_env ${extra_test_env} PARENT_SCOPE)
