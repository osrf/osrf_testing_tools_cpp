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
#include "./static_allocator.hpp"
#include "osrf_testing_tools_cpp/scope_exit.hpp"

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
  // fprintf(stderr, "original '%s()' is from '%s'\n", name, dl_info.dli_fname);
  return original_function;
}

using osrf_testing_tools_cpp::memory_tools::impl::StaticAllocator;
// the size was found experimentally on Ubuntu Linux 16.04 x86_64
using StaticAllocatorT = StaticAllocator<8388608>;
// used to fullfil calloc call from dlerror.c during initialization of original functions
// constructor is called on first use with a placement-new and the static storage
static uint8_t g_static_allocator_storage[sizeof(StaticAllocatorT)];
static StaticAllocatorT * g_static_allocator = nullptr;

// storage and initialization flags for original malloc/realloc/calloc/free
static bool g_original_functions_not_initialized = true;
using MallocSignature = void * (*)(size_t);
static MallocSignature g_original_malloc = nullptr;
using ReallocSignature = void *(*)(void *, size_t);
static ReallocSignature g_original_realloc = nullptr;
using CallocSignature = void *(*)(size_t, size_t);
static CallocSignature g_original_calloc = nullptr;
using FreeSignature = void (*)(void *);
static FreeSignature g_original_free = nullptr;

// on shared library load, find and store the original memory function locations
static void __linux_memory_tools_init(void) __attribute__((constructor));
static void __linux_memory_tools_init(void)
{
  g_original_malloc = find_original_function<MallocSignature>("malloc");
  g_original_realloc = find_original_function<ReallocSignature>("realloc");
  g_original_calloc = find_original_function<CallocSignature>("calloc");
  g_original_free = find_original_function<FreeSignature>("free");

  g_original_functions_not_initialized = false;
}

// used to ensure safe initialization of new threads
static __thread bool g_malloc_called_in_thread_tls = false;
static __thread bool g_in_malloc_first_thread_case = false;

// used to detect recursion
static __thread bool g_in_malloc_tls = false;
static __thread bool g_in_realloc_tls = false;
static __thread bool g_in_calloc_tls = false;
static __thread bool g_in_free_tls = false;

extern "C"
{

void *
malloc(size_t size) noexcept
{
  using osrf_testing_tools_cpp::memory_tools::custom_malloc_with_original;

  // This case covers the time before the global original functions are available.
  if (g_original_functions_not_initialized) {
    if (nullptr == g_static_allocator) {
      g_static_allocator = new (g_static_allocator_storage) StaticAllocatorT();
    }
    // SAFE_FWRITE(stderr, "StackAllocator.malloc(");
    // char tmp[256];
    // snprintf(tmp, sizeof(tmp), "%zu", size);
    // SAFE_FWRITE(stderr, tmp);
    // SAFE_FWRITE(stderr, ")\n");

    return g_static_allocator->allocate(size);
  }

  // These cases cover the first time malloc is used in a thread (perhaps in thread init).
  if (g_in_malloc_first_thread_case) {
    // In the process (malloc recursion) of doing first-time thread malloc, skip custom logic.
    return g_original_malloc(size);
  }
  if (!g_malloc_called_in_thread_tls) {
    // First time calling malloc in this thread.
    if (!g_in_malloc_first_thread_case) {
      // force a malloc in the thread to ensure first user malloc is not skipped
      g_in_malloc_first_thread_case = true;  // allows short circuit of malloc in the next call
      free(custom_malloc_with_original(32, g_original_malloc, false));
    }
    void * result = g_original_malloc(size);
    g_in_malloc_first_thread_case = false;
    return result;
  }

  // This case covers the situation where malloc is called from within custom_malloc_with_original.
  if (g_in_malloc_tls) {
    return g_original_malloc(size);
  }
  g_in_malloc_tls = true;
  auto unset_recursion_check = OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    g_in_malloc_tls = false;
  });

  g_malloc_called_in_thread_tls = true;
  return custom_malloc_with_original(size, g_original_malloc, false);
}

void *
realloc(void * pointer, size_t size) noexcept
{
  g_in_realloc_tls = true;
  auto unset_recursion_check = OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    g_in_realloc_tls = false;
  });

  if (
    g_original_functions_not_initialized &&
    g_static_allocator->pointer_belongs_to_allocator(pointer))
  {
    if (nullptr == g_static_allocator) {
      g_static_allocator = new (g_static_allocator_storage) StaticAllocatorT();
    }
    // SAFE_FWRITE(stderr, "StackAllocator.realloc(");
    // char tmp[256];
    // snprintf(tmp, sizeof(tmp), "%zu", size);
    // SAFE_FWRITE(stderr, tmp);
    // SAFE_FWRITE(stderr, ")\n");

    void * new_memory = g_static_allocator->allocate(size);
    if (!new_memory) {
      return nullptr;
    }
    if (!g_static_allocator->deallocate(reinterpret_cast<uint8_t *>(pointer))) {
      SAFE_FWRITE(stderr, "memory unexpectedly not loaned by static allocator\n");
    }
    return new_memory;
  }
  using osrf_testing_tools_cpp::memory_tools::custom_realloc_with_original;
  return custom_realloc_with_original(pointer, size, g_original_realloc, false);
}

void *
calloc(size_t count, size_t size) noexcept
{
  g_in_calloc_tls = true;
  auto unset_recursion_check = OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    g_in_calloc_tls = false;
  });

  if (g_original_functions_not_initialized) {
    if (nullptr == g_static_allocator) {
      g_static_allocator = new (g_static_allocator_storage) StaticAllocatorT();
    }
    // SAFE_FWRITE(stderr, "StackAllocator.calloc(");
    // char tmp[256];
    // snprintf(tmp, sizeof(tmp), "%zu", count * size);
    // SAFE_FWRITE(stderr, tmp);
    // SAFE_FWRITE(stderr, ")\n");

    return g_static_allocator->allocate(count * size);
  }
  using osrf_testing_tools_cpp::memory_tools::custom_calloc_with_original;
  return custom_calloc_with_original(count, size, g_original_calloc, false);
}

void
free(void * pointer) noexcept
{
  g_in_free_tls = true;
  auto unset_recursion_check = OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    g_in_free_tls = false;
  });

  if (g_static_allocator->deallocate(pointer)) {
    return;
  }
  if (g_in_malloc_first_thread_case) {
    return g_original_free(pointer);
  }
  using osrf_testing_tools_cpp::memory_tools::custom_free_with_original;
  custom_free_with_original(pointer, g_original_free, false);
}

}  // extern "C"

#endif  // defined(__linux__)
