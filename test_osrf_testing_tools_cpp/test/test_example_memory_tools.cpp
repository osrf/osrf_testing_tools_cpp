// Copyright 2015 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>

#include "osrf_testing_tools_cpp/memory_tools/testing_helpers.hpp"
#include "osrf_testing_tools_cpp/memory_tools/memory_tools.hpp"
#include "osrf_testing_tools_cpp/scope_exit.hpp"

/**
 * Tests the dynamic memory checking tools.
 */
TEST(TestMemoryTools, test_allocation_checking_tools) {
  osrf_testing_tools_cpp::memory_tools::initialize();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    osrf_testing_tools_cpp::memory_tools::uninitialize();
  });

  size_t unexpected_mallocs = 0;
  auto on_unexpected_malloc =
    [&unexpected_mallocs]() {
      unexpected_mallocs++;
    };
  osrf_testing_tools_cpp::memory_tools::on_unexpected_malloc(on_unexpected_malloc);
  size_t unexpected_reallocs = 0;
  auto on_unexpected_realloc =
    [&unexpected_reallocs]() {
      unexpected_reallocs++;
    };
  osrf_testing_tools_cpp::memory_tools::on_unexpected_realloc(on_unexpected_realloc);
  size_t unexpected_callocs = 0;
  auto on_unexpected_calloc =
    [&unexpected_callocs]() {
      unexpected_callocs++;
    };
  osrf_testing_tools_cpp::memory_tools::on_unexpected_calloc(on_unexpected_calloc);
  size_t unexpected_frees = 0;
  auto on_unexpected_free =
    [&unexpected_frees]() {
      unexpected_frees++;
    };
  osrf_testing_tools_cpp::memory_tools::on_unexpected_free(on_unexpected_free);
  void * mem = nullptr;
  void * remem = nullptr;

  auto use_all_dynamic_memory_functions = [&mem, &remem]() -> void {
    mem = malloc(1024);
    ASSERT_NE(nullptr, mem);
    remem = realloc(mem, 2048);
    ASSERT_NE(nullptr, remem);
    free(remem);
    mem = calloc(1024, sizeof(void *));
    ASSERT_NE(nullptr, mem);
    free(mem);
  };

  // First try before enabling, should have no effect.
  use_all_dynamic_memory_functions();
  EXPECT_EQ(0u, unexpected_mallocs);
  EXPECT_EQ(0u, unexpected_reallocs);
  EXPECT_EQ(0u, unexpected_callocs);
  EXPECT_EQ(0u, unexpected_frees);

  // Enable checking, but no assert, should have no effect.
  osrf_testing_tools_cpp::memory_tools::enable_monitoring();
  use_all_dynamic_memory_functions();
  EXPECT_EQ(0u, unexpected_mallocs);
  EXPECT_EQ(0u, unexpected_reallocs);
  EXPECT_EQ(0u, unexpected_callocs);
  EXPECT_EQ(0u, unexpected_frees);

  // Enable no_* asserts, should increment all once.
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_begin();
  osrf_testing_tools_cpp::memory_tools::expect_no_realloc_begin();
  osrf_testing_tools_cpp::memory_tools::expect_no_calloc_begin();
  osrf_testing_tools_cpp::memory_tools::expect_no_free_begin();
  use_all_dynamic_memory_functions();
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_end();
  osrf_testing_tools_cpp::memory_tools::expect_no_realloc_end();
  osrf_testing_tools_cpp::memory_tools::expect_no_calloc_end();
  osrf_testing_tools_cpp::memory_tools::expect_no_free_end();
  EXPECT_EQ(1u, unexpected_mallocs);
  EXPECT_EQ(1u, unexpected_reallocs);
  EXPECT_EQ(1u, unexpected_callocs);
  EXPECT_EQ(1u, unexpected_frees);

  // Enable on malloc assert, only malloc should increment.
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_begin();
  use_all_dynamic_memory_functions();
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_end();
  EXPECT_EQ(2u, unexpected_mallocs);
  EXPECT_EQ(1u, unexpected_reallocs);
  EXPECT_EQ(1u, unexpected_callocs);
  EXPECT_EQ(1u, unexpected_frees);

  // Enable on realloc assert, only realloc should increment.
  osrf_testing_tools_cpp::memory_tools::expect_no_realloc_begin();
  use_all_dynamic_memory_functions();
  osrf_testing_tools_cpp::memory_tools::expect_no_realloc_end();
  EXPECT_EQ(2u, unexpected_mallocs);
  EXPECT_EQ(2u, unexpected_reallocs);
  EXPECT_EQ(1u, unexpected_callocs);
  EXPECT_EQ(1u, unexpected_frees);

  // Enable on calloc assert, only calloc should increment.
  osrf_testing_tools_cpp::memory_tools::expect_no_calloc_begin();
  use_all_dynamic_memory_functions();
  osrf_testing_tools_cpp::memory_tools::expect_no_calloc_end();
  EXPECT_EQ(2u, unexpected_mallocs);
  EXPECT_EQ(2u, unexpected_reallocs);
  EXPECT_EQ(2u, unexpected_callocs);
  EXPECT_EQ(2u, unexpected_frees);

  // Enable on free assert, only free should increment.
  osrf_testing_tools_cpp::memory_tools::expect_no_free_begin();
  use_all_dynamic_memory_functions();
  osrf_testing_tools_cpp::memory_tools::expect_no_free_end();
  EXPECT_EQ(2u, unexpected_mallocs);
  EXPECT_EQ(2u, unexpected_reallocs);
  EXPECT_EQ(2u, unexpected_callocs);
  EXPECT_EQ(2u, unexpected_frees);

  // Go again, after disabling asserts, should have no effect.
  use_all_dynamic_memory_functions();
  EXPECT_EQ(2u, unexpected_mallocs);
  EXPECT_EQ(2u, unexpected_reallocs);
  EXPECT_EQ(2u, unexpected_callocs);
  EXPECT_EQ(2u, unexpected_frees);

  // Go once more after disabling everything, should have no effect.
  osrf_testing_tools_cpp::memory_tools::disable_monitoring();
  use_all_dynamic_memory_functions();
  EXPECT_EQ(2u, unexpected_mallocs);
  EXPECT_EQ(2u, unexpected_reallocs);
  EXPECT_EQ(2u, unexpected_callocs);
  EXPECT_EQ(2u, unexpected_frees);
}
