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

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "./parse_environment_variable.hpp"

void
usage(const std::string & program_name)
{
  printf(
    "usage: %s "
    "[--env ENV=VALUE [ENV2=VALUE [...]]] "
    "[--append-env ENV=VALUE [ENV2=VALUE [...]]] "
    "-- <command>\n", program_name.c_str());
}

bool
starts_with(const std::string & str, const std::string & prefix)
{
  return !str.compare(0, prefix.size(), prefix);
}

bool
starts_with_any(const std::string & str, const std::vector<const std::string> & prefixes)
{
  for (auto prefix : prefixes) {
    if (starts_with(str, prefix)) {
      return true;
    }
  }
  return false;
}

int
main(int argc, char const * argv[])
{
  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    if (i == 0) {
      continue;
    }
    args.emplace_back(argv[i]);
  }

  bool should_show_usage_and_exit = (args.size() <= 1);
  for (auto arg : args) {
    should_show_usage_and_exit |= starts_with_any(arg, {"-h", "--help"});
  }
  if (should_show_usage_and_exit) {
    usage(argv[0]);
    return 1;
  }

  std::map<std::string, std::string> env_variables;
  std::map<std::string, std::string> append_env_variables;
  std::vector<std::string> commands;

  std::string mode = "none";
  for (auto arg : args) {
    if (arg.empty()) {
      fprintf(stderr, "argument unexpectedly empty: %s\n", arg.c_str());
      return 1;
    }
    // once in command mode, consume all flags no matter the content
    if (mode == "command") {
      commands.push_back(arg);
      continue;
    }

    // determine if the mode needs to change
    if (starts_with(arg, "--env")) {
      mode = "env";
      continue;
    }
    if (starts_with(arg, "--append-env")) {
      mode = "append_env";
      continue;
    }
    if (arg == "--") {
      mode = "command";
      continue;
    }

    // determine where to store the argument
    if (mode == "none") {
      fprintf(stderr, "unexpected positional argument: %s\n", arg.c_str());
      usage(argv[0]);
      return 1;
    } else if (mode == "env") {
      try {
        auto env_pair = test_runner::parse_environment_variable(arg);
        env_variables[env_pair.first] = env_pair.second;
      } catch(const std::invalid_argument & exc) {
        fprintf(
          stderr,
          "invalid environment variable, expected ENV=VALUE, %s: %s\n",
          exc.what(), arg.c_str());
        return 1;
      }
    } else if (mode == "append_env") {
      try {
        auto env_pair = test_runner::parse_environment_variable(arg);
        append_env_variables[env_pair.first] = env_pair.second;
      } catch(const std::invalid_argument & exc) {
        fprintf(
          stderr,
          "invalid environment variable, expected ENV=VALUE, %s: %s\n",
          exc.what(), arg.c_str());
        return 1;
      }
    } else {
      fprintf(stderr, "unexpected mode '%s'\n", mode.c_str());
      return 1;
    }
  }

  // Set the environment variables.
  for (auto pair : env_variables) {
#if defined(_WIN32)
    if (0 != _putenv_s(pair.first.c_str(), pair.second.c_str())) {
#else
    if (0 != setenv(pair.first.c_str(), pair.second.c_str(), 1)) {
#endif
      fprintf(stderr,
        "failed to set environment variable '%s=%s'\n",
        pair.first.c_str(), pair.second.c_str());
      return 1;
    }
  }

    // Append the PATH-like environment variables.
  for (auto pair : append_env_variables) {
    const char * env_value = std::getenv(pair.first.c_str());
    if (!env_value) {
      env_value = "";
    }
    std::string new_value = env_value;
#if defined(_WIN32)
    auto path_sep = ";";
#else
    auto path_sep = ":";
#endif
    if (!new_value.empty() && new_value.substr(new_value.size() - 1) != path_sep) {
      new_value += path_sep;
    }
    new_value += pair.second;
#if defined(_WIN32)
    if (0 != _putenv_s(pair.first.c_str(), new_value.c_str())) {
#else
    if (0 != setenv(pair.first.c_str(), new_value.c_str(), 1)) {
#endif
      fprintf(stderr,
        "failed to set environment variable '%s=%s'\n",
        pair.first.c_str(), new_value.c_str());
      return 1;
    }
  }

  // Run the command.
  std::string command_str = "";
  for (auto command : commands) {
    command_str += command + " ";
  }
  if (!command_str.empty()) {
    command_str = command_str.substr(0, command_str.size() - 1);
  }
  int subprocess_return_code = 0;

#if defined(_WIN32)
  #define popen _popen
  #define pclose _pclose
#endif

  char buffer[256];
  FILE * pipe = popen(command_str.c_str(), "r");

  if (pipe == nullptr) {
    fprintf(stderr, "Failed to execute command '%s': %s\n", command_str.c_str(), strerror(errno));
    return 1;
  }

  while (fgets(buffer, sizeof(buffer), pipe)) {
    printf("%s", buffer);
  }

  if (feof(pipe)) {
    subprocess_return_code = pclose(pipe);
  } else {
    fprintf(stderr, "failed to read pipe until the end\n");
    return 1;
  }

  return subprocess_return_code;
}
