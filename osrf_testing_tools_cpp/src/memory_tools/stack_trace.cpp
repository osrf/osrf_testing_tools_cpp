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

#include "osrf_testing_tools_cpp/memory_tools/stack_trace.hpp"

#include "./stack_trace_impl.hpp"

namespace osrf_testing_tools_cpp
{
namespace memory_tools
{

SourceLocation::SourceLocation(std::shared_ptr<SourceLocationImpl> impl)
: impl_(std::move(impl))
{}

SourceLocation::~SourceLocation()
{}

const std::string &
SourceLocation::function() const
{
  return impl_->source_location->function;
}

const std::string &
SourceLocation::filename() const
{
  return impl_->source_location->filename;
}

size_t
SourceLocation::line() const
{
  return impl_->source_location->line;
}

size_t
SourceLocation::column() const
{
  return impl_->source_location->col;
}

Trace::Trace(std::unique_ptr<TraceImpl> impl)
: impl_(std::move(impl))
{}

Trace::Trace(const Trace & other)
: impl_(new TraceImpl(*other.impl_))
{}

Trace::~Trace()
{}

void *
Trace::address() const
{
  return impl_->resolved_trace.addr;
}

size_t
Trace::index_in_stack() const
{
  return impl_->resolved_trace.idx;
}

const std::string &
Trace::object_filename() const
{
  return impl_->resolved_trace.object_filename;
}

const std::string &
Trace::object_function() const
{
  return impl_->resolved_trace.object_function;
}

const SourceLocation &
Trace::source_location() const
{
  return impl_->source_location;
}

const std::vector<SourceLocation> &
Trace::inlined_source_locations() const
{
  return impl_->inlined_source_locations;
}

StackTrace::StackTrace(std::unique_ptr<StackTraceImpl> impl)
: impl_(std::move(impl))
{}

StackTrace::~StackTrace()
{}

std::thread::id
StackTrace::thread_id() const
{
  return impl_->thread_id;
}

const std::vector<Trace> &
StackTrace::get_traces() const
{
  return impl_->traces;
}

std::vector<Trace>
StackTrace::get_traces_from_function_name(const char * function_name) const
{
  std::vector<Trace> result;
  bool function_found = false;
  for (const Trace & trace : impl_->traces) {
    if (!function_found && trace.object_function().find(function_name) == 0) {
      function_found = true;
    }
    if (function_found) {
      result.emplace_back(trace);
    }
  }
  return result;
}

}  // namespace memory_tools
}  // namespace osrf_testing_tools_cpp
