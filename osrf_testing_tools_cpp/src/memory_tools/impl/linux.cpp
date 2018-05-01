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

#if defined(__linux__)

#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>

#include "../count_function_occurrences_in_backtrace.hpp"
#include "../custom_memory_functions.hpp"
#include "../implementation_initialization.hpp"

namespace osrf_testing_tools_cpp
{
namespace memory_tools
{

bool
implementation_specific_initialize()
{
  // call this once to get any one-time initializatin out of the way
  static_assert(
    count_function_occurrences_in_backtrace_is_implemented::value,
    "linux requires the ability to count_function_occurrences_in_backtrace()");
  (void)count_function_occurrences_in_backtrace(implementation_specific_initialize);
  return true;
}

void
implementation_specific_uninitialize() {}

}  // namespace memory_tools
}  // namespace osrf_testing_tools_cpp

template<typename FunctionPointerT>
FunctionPointerT
find_original_function(const char * name)
{
  FunctionPointerT original_function = reinterpret_cast<FunctionPointerT>(dlsym(RTLD_NEXT, name));
  if (!original_function) {
    fprintf(stderr, "failed to get original function '%s' with dlsym() and RTLD_NEXT\n", name);
    exit(1);  // cannot throw, next best thing
  }
  // check to make sure we got one we want, see:
  //   http://optumsoft.com/dangers-of-using-dlsym-with-rtld_next/
  Dl_info dl_info;
  if (!dladdr(reinterpret_cast<void *>(original_function), &dl_info)) {
    fprintf(stderr,
      "failed to get information about function '%p' with dladdr()\n",
      reinterpret_cast<void *>(original_function));
    exit(1);  // cannot throw, next best thing
  }
  fprintf(stderr, "%s is from %s\n", name, dl_info.dli_fname);
  return original_function;
}

// used to fullfil calloc call from dlerror.c during initialization of original functions
static uint8_t g_initialization_buffer[256];
static bool g_initialization_buffer_used = false;

static bool g_initializing_original_functions = true;
using MallocSignature = void * (*)(size_t);
static MallocSignature g_original_malloc = nullptr;
using ReallocSignature = void *(*)(void *, size_t);
static ReallocSignature g_original_realloc = nullptr;
using CallocSignature = void *(*)(size_t, size_t);
static CallocSignature g_original_calloc = nullptr;
using FreeSignature = void (*)(void *);
static FreeSignature g_original_free = nullptr;

static void __linux_memory_tools_init(void) __attribute__((constructor));
static void __linux_memory_tools_init(void)
{
  g_original_malloc = find_original_function<MallocSignature>("malloc");
  g_original_realloc = find_original_function<ReallocSignature>("realloc");
  g_original_calloc = find_original_function<CallocSignature>("calloc");
  g_original_free = find_original_function<FreeSignature>("free");

  g_initializing_original_functions = false;
}

extern "C"
{

void *
malloc(size_t size) noexcept
{
  if (g_initializing_original_functions) {
    return reinterpret_cast<MallocSignature>(dlsym(RTLD_NEXT, "malloc"))(size);
  }
  using osrf_testing_tools_cpp::memory_tools::custom_malloc_with_original;
  return custom_malloc_with_original(size, g_original_malloc);
}

void *
realloc(void * pointer, size_t size) noexcept
{
  using osrf_testing_tools_cpp::memory_tools::custom_realloc_with_original;
  return custom_realloc_with_original(pointer, size, g_original_realloc);
}

void *
calloc(size_t count, size_t size) noexcept
{
  if (g_initializing_original_functions) {
    if (count * size > sizeof(g_initialization_buffer)) {
      SAFE_FWRITE(stderr, "not enough space in buffer to satisfy some dlfnc call\n");
      exit(1);
    }
    if (g_initialization_buffer_used) {
      SAFE_FWRITE(stderr, "calloc called more than once during initialization\n");
      exit(1);
    }
    g_initialization_buffer_used = true;
    return g_initialization_buffer;
  }
  using osrf_testing_tools_cpp::memory_tools::custom_calloc_with_original;
  return custom_calloc_with_original(count, size, g_original_calloc);
}

void
free(void * pointer) noexcept
{
  if (pointer == g_initialization_buffer) {
    return;
  }
  using osrf_testing_tools_cpp::memory_tools::custom_free_with_original;
  custom_free_with_original(pointer, g_original_free);
}

}  // extern "C"

#endif  // defined(__linux__)
