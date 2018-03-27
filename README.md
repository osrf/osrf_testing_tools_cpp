#### osrf_testing_tools_cpp

This repository contains testing tools for C++, and is used in various OSRF projects.

##### memory_tools

This tool lets you intercept calls to dynamic memory calls like `malloc` and `free`, and provides some convenience functions for doing `googletest` assertions when code that is not supposed to allocate memory does so.
