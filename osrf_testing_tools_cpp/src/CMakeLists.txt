add_subdirectory(memory_tools)
add_subdirectory(test_runner)

set(memory_tools_extra_test_env "${memory_tools_extra_test_env}" PARENT_SCOPE)
set(memory_tools_is_available "${memory_tools_is_available}" PARENT_SCOPE)
