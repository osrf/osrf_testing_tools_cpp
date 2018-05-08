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

#include <cstdlib>

#include "../custom_memory_functions.hpp"

#if defined(__APPLE__)

// Pulled from:
//  https://github.com/emeryberger/Heap-Layers/blob/
//    076e9e7ef53b66380b159e40473b930f25cc353b/wrappers/macinterpose.h

// The interposition data structure (just pairs of function pointers),
// used an interposition table like the following:
//

typedef struct interpose_s
{
  void * new_func;
  void * orig_func;
} interpose_t;

#define OSX_INTERPOSE(newf, oldf) \
  __attribute__((used)) static const interpose_t \
  macinterpose__ ## newf ## __ ## oldf __attribute__ ((section("__DATA, __interpose"))) = { \
    reinterpret_cast<void *>(newf), \
    reinterpret_cast<void *>(oldf), \
  }

// End Interpose.

extern "C"
{

void *
replacement_malloc(size_t size)
{
  return osrf_testing_tools_cpp::memory_tools::custom_malloc(size);
}

void *
replacement_realloc(void * memory_in, size_t size)
{
  return osrf_testing_tools_cpp::memory_tools::custom_realloc(memory_in, size);
}

void *
replacement_calloc(size_t count, size_t size)
{
  return osrf_testing_tools_cpp::memory_tools::custom_calloc(count, size);
}

void
replacement_free(void * memory)
{
  osrf_testing_tools_cpp::memory_tools::custom_free(memory);
}

OSX_INTERPOSE(replacement_malloc, malloc);
OSX_INTERPOSE(replacement_realloc, realloc);
OSX_INTERPOSE(replacement_calloc, calloc);
OSX_INTERPOSE(replacement_free, free);

}  // extern "C"

#endif  // defined(__APPLE__)
