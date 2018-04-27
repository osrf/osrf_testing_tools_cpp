#### osrf_testing_tools_cpp

This repository contains testing tools for C++, and is used in OSRF projects.

##### memory_tools

This API lets you intercept calls to dynamic memory calls like `malloc` and `free`, and provides some convenience functions for differentiating between expected and unexpected calls to dynamic memory functions.

###### API Example

Here's a simple, googletest based example of how it is used:

```c++
#include <cstdlib>

#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#include "osrf_testing_tools_cpp/memory_tools/memory_tools.hpp"
#include "osrf_testing_tools_cpp/scope_exit.hpp"

void my_first_function()
{
  void * some_memory = std::malloc(1024);
  // .. do something with it
  std::free(some_memory);
}

int my_second_function(int a, int b)
{
  return a + b;
}

TEST(TestMemoryTools, test_example) {
  // you must initialize memory tools, but uninitialization is optional
  osrf_testing_tools_cpp::memory_tools::initialize();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    osrf_testing_tools_cpp::memory_tools::uninitialize();
  });

  // create a callback for "unexpected" mallocs that does a non-fatal gtest
  // failure and then register it with memory tools
  auto on_unexpected_malloc =
    []() {
      ADD_FAILURE() << "unexpected malloc";
    };
  osrf_testing_tools_cpp::memory_tools::on_unexpected_malloc(on_unexpected_malloc);

  // at this point, you'll still not get callbacks, since monitoring is not enabled
  // so calling either user function should not fail
  my_first_function();
  EXPECT_EQ(my_second_function(1, 2), 3);

  // enabling monitoring will allow checking to begin, but the default state is
  // that dynamic memory calls are expected, so again either function will pass
  osrf_testing_tools_cpp::memory_tools::enable_monitoring();
  my_first_function();
  EXPECT_EQ(my_second_function(1, 2), 3);

  // if you then tell memory tools that malloc is unexpected, then it will call
  // your above callback, at least until you indicate malloc is expected again
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_begin();
  EXPECT_NONFATAL_FAILURE(my_first_function(), "unexpected malloc");
  EXPECT_EQ(my_second_function(1, 2), 3);
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_end();
}
```
