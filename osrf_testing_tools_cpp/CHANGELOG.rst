^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package osrf_testing_tools_cpp
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

2.0.0 (2024-02-07)
------------------
* Upgrade to Google test 1.14.0 (`#84 <https://github.com/osrf/osrf_testing_tools_cpp/issues/84>`_)
* Contributors: Chris Lalancette

1.7.0 (2023-04-28)
------------------

1.5.3 (2023-04-11)
------------------
* Fix mpark/variant conditional for MSVC (`#77 <https://github.com/osrf/osrf_testing_tools_cpp/issues/77>`_)
* Changing C++ Compile Version (`#76 <https://github.com/osrf/osrf_testing_tools_cpp/issues/76>`_)
* Update maintainers (`#74 <https://github.com/osrf/osrf_testing_tools_cpp/issues/74>`_)
* Contributors: Audrow Nash, Lucas Wendland, Scott K Logan

1.5.2 (2022-11-07)
------------------
* Sets CMP0135 policy behavior to NEW (`#73 <https://github.com/osrf/osrf_testing_tools_cpp/issues/73>`_)
* Fixes policy CMP0135 warning in CMake 3.24 (`#71 <https://github.com/osrf/osrf_testing_tools_cpp/issues/71>`_)
* Add cstring include. (`#70 <https://github.com/osrf/osrf_testing_tools_cpp/issues/70>`_)
* Contributors: Chris Lalancette, Cristóbal Arroyo

1.5.1 (2022-02-14)
------------------
* Changed to use ``CMAKE_DL_LIBS`` CMake library to link library that provides dlopen (`#68 <https://github.com/osrf/osrf_testing_tools_cpp/issues/68>`_)
* Contributors: Silvio Traversaro

1.5.0 (2022-01-14)
------------------
* Update backward-cpp to latest master commit (`#64 <https://github.com/osrf/osrf_testing_tools_cpp/issues/64>`_)
* Patch googletest to 1.10.0.2 (`#65 <https://github.com/osrf/osrf_testing_tools_cpp/issues/65>`_)
* Contributors: Christophe Bedard, Homalozoa X

1.4.0 (2020-12-08)
------------------
* [osrf_testing_tools_cpp] Add warnings (`#54 <https://github.com/osrf/osrf_testing_tools_cpp/issues/54>`_)
* Update cmake minimum version to 2.8.12 (`#61 <https://github.com/osrf/osrf_testing_tools_cpp/issues/61>`_)
* Add googletest v1.10.0 (`#55 <https://github.com/osrf/osrf_testing_tools_cpp/issues/55>`_)
* Workarounds for Android (`#52 <https://github.com/osrf/osrf_testing_tools_cpp/issues/52>`_) (`#60 <https://github.com/osrf/osrf_testing_tools_cpp/issues/60>`_)
* Change `WIN32` to `_WIN32` (`#53 <https://github.com/osrf/osrf_testing_tools_cpp/issues/53>`_)
* fix execinfo.h not found for QNX (`#50 <https://github.com/osrf/osrf_testing_tools_cpp/issues/50>`_)
* Contributors: Ahmed Sobhy, Audrow Nash, Dan Rose, Jacob Perron, Stephen Brawner

1.3.2 (2020-05-21)
------------------
* Suppressed a FPHSA CMake warning in Backward (`#48 <https://github.com/osrf/osrf_testing_tools_cpp/issues/48>`_)
* Contributors: Dirk Thomas
