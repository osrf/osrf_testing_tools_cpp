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
#include "../implementation_initialization.hpp"

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
  macinterpose ## newf ## oldf __attribute__ ((section("__DATA, __interpose"))) = { \
    reinterpret_cast<void *>(newf), \
    reinterpret_cast<void *>(oldf), \
  }

// End Interpose.

using osrf_testing_tools_cpp::memory_tools::custom_malloc;
using osrf_testing_tools_cpp::memory_tools::custom_realloc;
using osrf_testing_tools_cpp::memory_tools::custom_calloc;
using osrf_testing_tools_cpp::memory_tools::custom_free;

OSX_INTERPOSE(custom_malloc, malloc);
OSX_INTERPOSE(custom_realloc, realloc);
OSX_INTERPOSE(custom_calloc, calloc);
OSX_INTERPOSE(custom_free, free);

namespace osrf_testing_tools_cpp
{
namespace memory_tools
{

bool
implementation_specific_initialize()
{
  return true;
}

void
implementation_specific_uninitialize() {}


}  // namespace memory_tools
}  // namespace osrf_testing_tools_cpp

#endif  // defined(__APPLE__)
