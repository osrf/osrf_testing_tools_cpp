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

#ifndef OSRF_TESTING_TOOLS_CPP__VARIANT_HELPER_HPP_
#define OSRF_TESTING_TOOLS_CPP__VARIANT_HELPER_HPP_

#if defined(__has_include) && __has_include(<variant>)

#include <variant>

#else  // defined(__has_include) && __has_include(<variant>)

// This is a header-only version of std::variant (part of C++17) for C++14.
// In the future this could be replaced with #include <variant>.
#include "./vendor/mpark/variant/variant.hpp"

namespace std
{

using namespace mpark;  // NOLINT(build/namespaces)

}  // namespace std

#endif  // defined(__has_include) && __has_include(<variant>)

#endif  // OSRF_TESTING_TOOLS_CPP__VARIANT_HELPER_HPP_
