// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#ifndef MEMORY_TOOLS__IMPL__STATIC_ALLOCATOR_HPP_
#define MEMORY_TOOLS__IMPL__STATIC_ALLOCATOR_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>

#include "../safe_fwrite.hpp"

namespace osrf_testing_tools_cpp
{
namespace memory_tools
{
namespace impl
{

template<std::size_t MemoryPoolSize>
class StaticAllocator
{
public:
  StaticAllocator()
  : memory_pool_{0},
    begin_(memory_pool_),
    end_(memory_pool_ + MemoryPoolSize),
    stack_pointer_(memory_pool_)
  {
    (void)memset(memory_pool_, 0x0, sizeof(memory_pool_));
  }

  void *
  allocate(std::size_t size)
  {
    if (size <= std::size_t(std::distance(stack_pointer_, end_))) {
      uint8_t * result = stack_pointer_;
      stack_pointer_ += size;
      return result;
    }
    SAFE_FWRITE(stderr, "StackAllocator.allocate() -> nullptr\n");
    return nullptr;
  }

  bool
  pointer_belongs_to_allocator(const void * pointer) const
  {
    const uint8_t * typed_pointer = reinterpret_cast<const uint8_t *>(pointer);
    return (
      !(std::less<const uint8_t *>()(typed_pointer, begin_)) &&
      (std::less<const uint8_t *>()(typed_pointer, end_))
    );
  }

  bool
  deallocate(void * pointer)
  {
    return this->pointer_belongs_to_allocator(pointer);
  }

private:
  uint8_t memory_pool_[MemoryPoolSize];
  uint8_t * begin_;
  uint8_t * end_;
  uint8_t * stack_pointer_;
};

}  // namespace impl
}  // namespace memory_tools
}  // namespace osrf_testing_tools_cpp

int main(void)
{
  osrf_testing_tools_cpp::memory_tools::impl::StaticAllocator<64> sa;
  void * mem = sa.allocate(16);
  (void)mem;
  return 0;
}

#endif  // MEMORY_TOOLS__IMPL__STATIC_ALLOCATOR_HPP_
