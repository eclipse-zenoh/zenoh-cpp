<img src="https://raw.githubusercontent.com/eclipse-zenoh/zenoh/master/zenoh-dragon.png" height="150">

[![CI](https://github.com/eclipse-zenoh/zenoh-cpp/workflows/CI/badge.svg)](https://github.com/eclipse-zenoh/zenoh-cpp/actions?query=workflow%3A%22CI%22)
[![Documentation Status](https://readthedocs.org/projects/zenoh-cpp/badge/?version=latest)](https://zenoh-cpp.readthedocs.io/en/latest/?badge=latest)
[![Discussion](https://img.shields.io/badge/discussion-on%20github-blue)](https://github.com/eclipse-zenoh/roadmap/discussions)
[![Discord](https://img.shields.io/badge/chat-on%20discord-blue)](https://discord.gg/2GJ958VuHs)
[![License](https://img.shields.io/badge/License-EPL%202.0-blue)](https://choosealicense.com/licenses/epl-2.0/)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# Eclipse Zenoh

The Eclipse Zenoh: Zero Overhead Pub/sub, Store/Query and Compute.

Zenoh (pronounce _/zeno/_) unifies data in motion, data at rest and computations. It carefully blends traditional pub/sub with geo-distributed storages, queries and computations, while retaining a level of time and space efficiency that is well beyond any of the mainstream stacks.

Check the website [zenoh.io](http://zenoh.io) and the [roadmap](https://github.com/eclipse-zenoh/roadmap) for more detailed information.

# C++ API

The Zenoh C++ API are headers only C++ bindings for [zenoh-c] and [zenoh-pico] libraries.

C++ bindings are still under active development so the Zenoh team will highly appreciate any help in testing them on various platforms, system architecture, etc. and to report any issue you might encounter. This will help in greatly improving its maturity and robustness.

## Requirements

The only hard requirement for building the library is a C++17-compliant compiler.
Using the library requires either [zenoh-c] or [zenoh-pico] to be installed.

-------------------------------

## How to build and install it

To install [zenoh-cpp] do the following steps:

1. Clone the sources.

   ```bash
   git clone https://github.com/eclipse-zenoh/zenoh-cpp.git
   ```

2. Build and install.

   By default it is expected that you have [zenoh-c] installed. If you want to install for [zenoh-pico] backend or for both (or to not specify any backend), please set `ZENOHCXX_ZENOHC` or `ZENOHCXX_ZENOHPICO` Cmake variables to`ON` or `OFF` accordingly. Notice that at least one of the backends is required for using the library and/or building tests and examples.

   Use option `CMAKE_INSTALL_PREFIX` for specifying installation location. Without this parameter installation is performed to default system location `/usr/local` which requires root privileges.

    ```bash
    mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=~/.local # to configure only for zenoh-c backend
    cmake .. -DZENOHCXX_ZENOHC=OFF -DZENOHCXX_ZENOHPICO=ON  -DCMAKE_INSTALL_PREFIX=~/.local # to configure  only for zenoh-pico backend
    cmake .. -DZENOHCXX_ZENOHC=OFF -DZENOHCXX_ZENOHPICO=OFF  -DCMAKE_INSTALL_PREFIX=~/.local # to configure for none of the backends
    cmake .. -DZENOHCXX_ZENOHPICO=ON  -DCMAKE_INSTALL_PREFIX=~/.local # to configure for both backends
    cmake --install .
    ```

## Building and running tests

By default it is expected that you have [zenoh-c] installed. If you want to build and run tests for [zenoh-pico] backend or for both, please set `ZENOHCXX_ZENOHC` or `ZENOHCXX_ZENOHPICO` Cmake variables to`ON` or `OFF` accordingly.

To build tests run:

```bash
mkdir build && cd build
cmake ..  -DCMAKE_INSTALL_PREFIX=~/.local # to build tests only for zenoh-c backend
cmake .. -DZENOHCXX_ZENOHC=OFF -DZENOHCXX_ZENOHPICO=ON  -DCMAKE_INSTALL_PREFIX=~/.local # to build tests only for zenoh-pico backend
cmake .. -DZENOHCXX_ZENOHPICO=ON  -DCMAKE_INSTALL_PREFIX=~/.local # to build tests for both backends
cmake --build . --target tests
ctest
```

Notice that the output of `cmake ../zenoh-cpp` shows where [zenoh-c] and/or [zenoh-pico] the dependencies were found.

## Building the Examples

Examples are splitted into two subdirectories. Subdirectory `universal` contains [zenoh-cpp] examples buildable with both [zenoh-c] and [zenoh-pico] backends. The `zenohc` subdirectory contains examples with zenoh-c specific functionality.

By default it is expected that you have [zenoh-c] installed. If you want to build examples for [zenoh-pico] backend or for both, please set `ZENOHCXX_ZENOHC` or `ZENOHCXX_ZENOHPICO` Cmake variables to`ON` or `OFF` accordingly.

To build examples run:

```bash
cmake ..  -DCMAKE_INSTALL_PREFIX=~/.local # to build examples only for zenoh-c backend
cmake .. -DZENOHCXX_ZENOHC=OFF -DZENOHCXX_ZENOHPICO=ON  -DCMAKE_INSTALL_PREFIX=~/.local # to build examples only for zenoh-pico backend
cmake .. -DZENOHCXX_ZENOHPICO=ON  -DCMAKE_INSTALL_PREFIX=~/.local # to build examples for both backends
cmake --build . --target examples
```

Examples are placed into `build/examples/zenohc` and `build/examples/zenohpico` directories.

## Running the examples

See information about running examples [here](./examples/README.md).

Examples of linking [zenoh-cpp] to an external project can be found [here](./examples/simple/Readme.md).

## Library usage

Below are the steps to include [zenoh-cpp] into CMake project. See also [examples/simple](examples/simple) directory for short examples of CMakeLists.txt.

- include [zenoh-c] or [zenoh-pico] into your CMake project **before** dependency on [zenoh-cpp] itself.
  This is important as the library targets you need (`zenohcxx::zenohpico`, `zenohcxx::zenohc::lib`) are defined only if their backend library targets (`zenohpico::lib` and/or `zenohc::lib` are defined)

- include [zenoh-cpp] using [find_package] CMake function:

  ```cmake
  find_package(zenohc) #if using zenoh-c backend
  find_package(zenohpico) #if using zenoh-pico backend
  find_package(zenohcxx)
  ```

- add dependency on zenoh-cpp to your project:

  ```cmake
  target_link_libraries(yourproject PUBLIC zenohcxx::zenohc) #if using zenoh-c backend
  target_link_libraries(yourproject PUBLIC zenohcxx::zenohpico) #if using zenoh-pico backend
  ```

- include the [zenoh.hxx] header. All zenoh functionality is available under the namespace `zenoh`:

  ```c++
  #include "zenoh.hxx"
  using namespace zenoh;
  ```

### Documentation

The documentation is on [zenoh-cpp.readthedocs.io].
Instruction how to build documentation locally is at [docs/README.md].

[rust-lang](https://www.rust-lang.org)
[zenoh](https://github.com/eclipse-zenoh/zenoh)
[zenoh-c](https://github.com/eclipse-zenoh/zenoh-c)
[zenoh-cpp](https://github.com/eclipse-zenoh/zenoh-cpp)
[zenoh-pico](https://github.com/eclipse-zenoh/zenoh-pico)
[zenoh.hxx](https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/include/zenoh.hxx)
[add_subdirectory](https://cmake.org/cmake/help/latest/command/add_subdirectory.html)
[find_package](https://cmake.org/cmake/help/latest/command/find_package.html)
[FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)
[zenoh-cpp.readthedocs.io](https://zenoh-cpp.readthedocs.io)
[docs/README.md](https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/docs/README.md)
