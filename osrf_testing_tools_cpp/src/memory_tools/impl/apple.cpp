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

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <set>
#include <thread>

#include "../custom_memory_functions.hpp"
#include "osrf_testing_tools_cpp/scope_exit.hpp"

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

static bool g_original_functions_initialized = false;

// on shared library load, find and store the original memory function locations
static void __apple_memory_tools_init(void) __attribute__((constructor));
static void __apple_memory_tools_init(void)
{
  g_original_functions_initialized = true;
}

struct SpinLock
{
  std::atomic_flag locked = ATOMIC_FLAG_INIT;
  void lock() {while (locked.test_and_set(std::memory_order_acquire)) {}}
  void unlock() {locked.clear(std::memory_order_release);}
};

// static std::set<std::thread::id> g_initialized_thread_ids;
static std::set<size_t> g_initialized_thread_ids;
static SpinLock g_spin_lock;
static std::thread::id g_thread_id_being_initialized;
static bool g_master_short_circuit = false;

static thread_local size_t g_thread_index_tls = 0;

void
assign_thread_index(size_t thread_index)
{
  g_thread_index_tls = thread_index;
}

size_t
get_assigned_thread_index()
{
  return g_thread_index_tls;
}

bool
thread_has_been_initialized()
{
  // return std::find(
  //   g_initialized_thread_ids.begin(), g_initialized_thread_ids.end(),
  //   std::this_thread::get_id()) != g_initialized_thread_ids.end();
  size_t thread_index = 0;
  // if it's either empty or only contains index 0, then it's the main thread
  if (g_initialized_thread_ids.size() > 1) {
    // this is not main (first) thread
    thread_index = get_assigned_thread_index();
  }
  return std::find(
    g_initialized_thread_ids.begin(), g_initialized_thread_ids.end(),
    thread_index) != g_initialized_thread_ids.end();
}

void
reverse_str(char * str, size_t count)
{
  size_t i = 0;
  do {
    char left = str[i];
    str[i] = str[count - i];
    str[count - i] = left;
    ++i;
  } while (i <= count / 2);
}

template<typename InputType>
size_t
unsigned_integer_to_str(InputType input, char * output, size_t output_size)
{
  size_t output_index = 0;
  do {
    output[output_index++] = '0' + (input % 10);
    input /= 10;
  } while (input != 0 && output_index < output_size - 1);
  output[output_index] = '\0';
  reverse_str(output, output_index - 1);
  return output_index;
};

void
print_thread_id()
{
  // const std::thread::id thread_id = std::this_thread::get_id();

  // char tmp[256];
  // size_t hash = std::hash<std::thread::id>()(thread_id);
  // unsigned_integer_to_str(hash, tmp, sizeof(tmp));
  mach_port_t tid = pthread_mach_thread_np(pthread_self());
  char tmp2[256];
  unsigned_integer_to_str(tid, tmp2, sizeof(tmp2));

  // SAFE_FWRITE(stderr, tmp);
  // SAFE_FWRITE(stderr, " ");
  SAFE_FWRITE(stderr, tmp2);
}

// used to remove thread from g_initialized_thread_ids when exiting
struct ThreadCleanup
{
  ThreadCleanup() : noop(false) {}
  ~ThreadCleanup()
  {
    g_spin_lock.lock();
    auto thread_id = std::this_thread::get_id();
    g_thread_id_being_initialized = thread_id;
    SAFE_FWRITE(stderr, "thread with id ");
    print_thread_id();
    SAFE_FWRITE(stderr, " is exiting\n");
    // g_initialized_thread_ids.erase(thread_id);
    g_initialized_thread_ids.erase(get_assigned_thread_index());
    SAFE_FWRITE(stderr, "thread with id ");
    print_thread_id();
    SAFE_FWRITE(stderr, " finished exiting\n");
    g_thread_id_being_initialized = std::thread::id();
    g_spin_lock.unlock();
  }

  bool noop;
};
// must use thread_local here (rather than __thread) for type with non-trivial destructor
static thread_local ThreadCleanup g_used_only_for_destructor_tls;

// used to detect recursion
static __thread bool g_in_free_tls = false;
static __thread bool g_in_malloc_tls = false;
static __thread bool g_in_realloc_tls = false;
static __thread bool g_in_calloc_tls = false;

// enum class FunctionType {malloc, realloc, calloc, free};
struct MallocFunction {static bool & get_flag() {return g_in_malloc_tls;}};
struct ReallocFunction {static bool & get_flag() {return g_in_realloc_tls;}};
struct CallocFunction {static bool & get_flag() {return g_in_calloc_tls;}};
struct FreeFunction {static bool & get_flag() {return g_in_free_tls;}};

template<typename FunctionType>
bool
within_function()
{
  return FunctionType::get_flag();
}

template<typename FunctionType>
struct ScopedIn
{
  ScopedIn() {FunctionType::get_flag() = true;}
  ~ScopedIn() {FunctionType::get_flag() = false;}
};

bool
handle_thread_initialization()
{
  const std::thread::id thread_id = std::this_thread::get_id();

  if (g_master_short_circuit || g_thread_id_being_initialized == thread_id) {
    SAFE_FWRITE(stderr,
      "in handle_thread_initialization(), during initialization for thread-id ");
    print_thread_id();
    SAFE_FWRITE(stderr, "\n");
    return true;
  }
  g_spin_lock.lock();
  if (!thread_has_been_initialized()) {
    SAFE_FWRITE(stderr,
      "in handle_thread_initialization(), inside thread_has_been_initialized(), for thread-id ");
    print_thread_id();
    SAFE_FWRITE(stderr, "\n");
    g_thread_id_being_initialized = thread_id;
    // g_initialized_thread_ids.insert(thread_id);
    g_initialized_thread_ids.insert(get_assigned_thread_index());
    {
      // set thread-local storage explicitly to ensure it is initialized
      ScopedIn<MallocFunction> scoped_in_malloc;
      (void)within_function<MallocFunction>();
      ScopedIn<ReallocFunction> scoped_in_realloc;
      (void)within_function<ReallocFunction>();
      ScopedIn<CallocFunction> scoped_in_calloc;
      (void)within_function<CallocFunction>();
      ScopedIn<FreeFunction> scoped_in_free;
      (void)within_function<FreeFunction>();
    }
    g_used_only_for_destructor_tls.noop = true;
    g_thread_id_being_initialized = std::thread::id();
  }
  g_spin_lock.unlock();
  return false;
}

extern "C"
{

struct ThreadCreateHookStorage
{
  void * (*start)(void *);
  void * arg;
  size_t assigned_thread_index;
  std::thread::id started_thread_id;
  mach_port_t tid;
  std::atomic<bool> id_is_set;
};

void *
pthread_create_helper(void * untyped_thread_create_hook_storage)
{
  auto thread_create_hook_storage =
    reinterpret_cast<ThreadCreateHookStorage *>(untyped_thread_create_hook_storage);
  SAFE_FWRITE(stderr, "in pthread_create_helper() with assigned_thread_index of ");
  char tmp[256];
  unsigned_integer_to_str(thread_create_hook_storage->assigned_thread_index, tmp, sizeof(tmp));
  SAFE_FWRITE(stderr, tmp);
  SAFE_FWRITE(stderr, "\n");
  thread_create_hook_storage->started_thread_id = std::this_thread::get_id();
  thread_create_hook_storage->tid = pthread_mach_thread_np(pthread_self());
  thread_create_hook_storage->id_is_set.store(true);
  std::this_thread::yield();
  {
    g_spin_lock.lock();
    g_master_short_circuit = true;
    g_thread_id_being_initialized = thread_create_hook_storage->started_thread_id;
    assign_thread_index(thread_create_hook_storage->assigned_thread_index);
    g_thread_id_being_initialized = std::thread::id();
    g_master_short_circuit = false;
    g_spin_lock.unlock();
    handle_thread_initialization();
  }
  if (thread_create_hook_storage->start) {
    return thread_create_hook_storage->start(thread_create_hook_storage->arg);
  }
  static int failed = -1;
  return &failed;
}

int
replacement_pthread_create(
  pthread_t * thread,
  pthread_attr_t * attr,
  void * (*start)(void *),
  void * arg)
{
  static size_t thread_index = 1;  // 0 is special, uninitialized value
  size_t this_thread_index = thread_index++;
  SAFE_FWRITE(stderr, "started creating thread number ");
  char tmp[256];
  unsigned_integer_to_str(this_thread_index, tmp, sizeof(tmp));
  SAFE_FWRITE(stderr, tmp);
  SAFE_FWRITE(stderr, "\n");
  ThreadCreateHookStorage custom_arg;
  custom_arg.start = start;
  custom_arg.arg = arg;
  custom_arg.assigned_thread_index = thread_index;
  custom_arg.started_thread_id = std::thread::id();
  custom_arg.id_is_set.store(false);
  int result = pthread_create(thread, attr, pthread_create_helper, &custom_arg);
  if (0 != result) {
    return result;
  }
  while (!custom_arg.id_is_set) {
    std::this_thread::yield();
  }
  SAFE_FWRITE(stderr, "finished creating thread number ");
  SAFE_FWRITE(stderr, tmp);
  SAFE_FWRITE(stderr, " which has thread id ");
  // size_t thread_id = std::hash<std::thread::id>()(custom_arg.started_thread_id);
  // unsigned_integer_to_str(thread_id, tmp, sizeof(tmp));
  unsigned_integer_to_str(custom_arg.tid, tmp, sizeof(tmp));
  SAFE_FWRITE(stderr, tmp);
  SAFE_FWRITE(stderr, "\n");
  return result;
}

int
replacement_pthread_join(pthread_t thread, void ** return_value)
{
  SAFE_FWRITE(stderr, "started joining thread number ");
  char tmp[256];
  unsigned_integer_to_str(0, tmp, sizeof(tmp));
  SAFE_FWRITE(stderr, tmp);
  SAFE_FWRITE(stderr, "\n");
  return pthread_join(thread, return_value);
}

#define RECURSION_DETECTED \
  __builtin_return_address(0) == __builtin_return_address(1) || \
  __builtin_return_address(0) == __builtin_return_address(2) || \
  __builtin_return_address(0) == __builtin_return_address(3) || \
  __builtin_return_address(0) == __builtin_return_address(4) || \
  __builtin_return_address(0) == __builtin_return_address(5) || \
  __builtin_return_address(0) == __builtin_return_address(6) || \
  __builtin_return_address(0) == __builtin_return_address(7) || \
  __builtin_return_address(0) == __builtin_return_address(8) || \
  __builtin_return_address(0) == __builtin_return_address(9)

void *
replacement_malloc(size_t size)
{
  if (RECURSION_DETECTED) {
    return malloc(size);
  }
  const std::thread::id thread_id = std::this_thread::get_id();
  (void)thread_id;
  // This condition helps avoid issues when dyld uses malloc.
  if (!g_original_functions_initialized) {
    return malloc(size);
  }

  // This section helps with thread-local storage initialization.
  if (handle_thread_initialization()) {
    return malloc(size);
  }

  // This condition prevents infinite recursion in custom_malloc_with_original().
  if (within_function<MallocFunction>()) {
    return malloc(size);
  }
  ScopedIn<MallocFunction> scoped_in_malloc;

  return osrf_testing_tools_cpp::memory_tools::custom_malloc_with_original(size, malloc, false);
}

void *
replacement_realloc(void * memory_in, size_t size)
{
  // This condition helps avoid issues when dyld uses realloc.
  if (!g_original_functions_initialized) {
    return realloc(memory_in, size);
  }

  // This condition prevents infinite recursion in custom_realloc_with_original().
  if (within_function<ReallocFunction>()) {
    return realloc(memory_in, size);
  }
  ScopedIn<ReallocFunction> scoped_in_realloc;

  using osrf_testing_tools_cpp::memory_tools::custom_realloc_with_original;
  return custom_realloc_with_original(memory_in, size, realloc, false);
}

void *
replacement_calloc(size_t count, size_t size)
{
  // This condition helps avoid issues when dyld uses calloc.
  if (!g_original_functions_initialized) {
    return calloc(count, size);
  }

  // This condition prevents infinite recursion in custom_calloc_with_original().
  if (within_function<CallocFunction>()) {
    return calloc(count, size);
  }
  ScopedIn<CallocFunction> scoped_in_calloc;

  using osrf_testing_tools_cpp::memory_tools::custom_calloc_with_original;
  return custom_calloc_with_original(count, size, calloc, false);
}

void
replacement_free(void * memory)
{
  if (RECURSION_DETECTED) {
    return free(memory);
  }
  // This condition helps avoid issues when dyld uses free.
  if (!g_original_functions_initialized) {
    return free(memory);
  }

  // This section helps with thread-local storage initialization.
  if (handle_thread_initialization()) {
    return free(memory);
  }

  // This condition prevents infinite recursion in custom_free_with_original().
  if (within_function<FreeFunction>()) {
    return free(memory);
  }
  ScopedIn<FreeFunction> scoped_in_free;

  osrf_testing_tools_cpp::memory_tools::custom_free_with_original(memory, free, false);
}

OSX_INTERPOSE(replacement_malloc, malloc);
OSX_INTERPOSE(replacement_realloc, realloc);
OSX_INTERPOSE(replacement_calloc, calloc);
OSX_INTERPOSE(replacement_free, free);
OSX_INTERPOSE(replacement_pthread_create, pthread_create);
OSX_INTERPOSE(replacement_pthread_join, pthread_join);

}  // extern "C"

#endif  // defined(__APPLE__)
