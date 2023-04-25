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

The Zenoh C++ API is a C++ bindings for [zenoh-c] and [zenoh-pico] libraries. The library is headers only.


C++ bindings are still so the Zenoh team will highly appreciate any help in testing them on various platforms, system architecture, etc. and to report any issue you might encounter. This will help in greatly improving its maturity and robustness.

-------------------------------
## How to build and install it 

> :warning: **WARNING** :warning: : Zenoh and its ecosystem are under active development. When you build from git, make sure you also build from git any other Zenoh repository you plan to use (e.g. binding, plugin, backend, etc.). It may happen that some changes in git are not compatible with the most recent packaged Zenoh release (e.g. deb, docker, pip). We put particular effort in mantaining compatibility between the various git repositories in the Zenoh project.

To install [zenoh-cpp] do the following steps:

1. Clone the sources

   ```bash
   git clone https://github.com/eclipse-zenoh/zenoh-cpp.git
   ```

2. Do install. 
   Neither [zenoh-c] nor [zenoh-pico] are required for the installation, but both are neccessary for building tests and examples. So, instead of the main project, it's faster to do install from "install" subproject.

   Use option `CMAKE_INSTALL_PREFIX` for specifying installation location. Without this parameter installation is performed to default system location `/usr/local` which requires root privileges.

    ```bash
    mkdir build && cd build
    cmake ../zenoh-cpp/install -DCMAKE_INSTALL_PREFIX=~/.local
    cmake --install .
    ```

## Building and running tests

```bash
mkdir -p build && cd build 
cmake ../zenoh-cpp
cmake --build . --target tests
ctest
```

## Building the Examples

Examples are splitted into two subdirectories. Subdirectory `universal` contains examples buildable with both [zenoh-c] and [zenoh-pico]. The `zenohc` subdirectory contains examples with zenoh-c specific functionality. The name postfix for CMake targets are `zenohc` and `zenohpico`. I.e. target `z_get_zenohpico` builds example `examples/universal/z_get.cxx` with `zenohpico` library into `build\examples\zenohpico\z_get`.

The examples can be built in two ways. One is to select `examples` as a build target of the main project (assuming here that the current directory is side-by-side with zenoh-cpp directory):

```bash
$ cmake ../zenoh-cpp
$ cmake --build . --target examples
```

Second way is to build `examples` as a root project. This demonstrates the ways to add dependency from zenoh-cpp into customer project.

```bash
$ cmake ../zenoh-cpp/examples
$ cmake --build .
```

You may also use `--target <example_name>` if you wish to only build a specific example (see target names convention above).

## Running the Examples

Change current directory to the variant you want (`examples/zenohc` or `examples/zenohpico` in the build directory)

### Basic Pub/Sub Example
```bash
./z_sub_cpp
```

```bash
./z_pub_cpp
```

### Queryable and Query Example
```bash
./z_queryable_cpp
```

```bash
./z_get_cpp
```

### Throughput Examples
```bash
./z_sub_thgr_cpp
```

```bash
./z_pub_thgr_cpp
```

## Library usage

Below are the steps to include [zenoh-cpp] into CMake project. See also [examples/simple](examples/simple) directory for short examples of CMakeLists.txt. 

- include zenoh-cpp into your CMake project. This can be done with [add_subdirectory], [find_package] or [FetchContent] CMake commands.
  ```
  add_subdirectory(../zenoh-cpp)
  ```
  ```
  find_package(zenohcxx)
  ```
  ```
  FetchContent_Declare(zenohcxx GIT_REPOSITORY https://github.com/eclipse-zenoh/zenoh-cpp GIT_TAG main)
  FetchContent_MakeAvailable(zenohcxx)
  ```
- include [zenoh-c] or [zenoh-pico] into your CMake project in the same way

- add dependency on zenoh-cpp to your project:
   ```
   target_link_libraries(yourproject PUBLIC zenohcxx::zenohpico)
   ```
   ```
   target_link_libraries(yourproject PUBLIC zenohcxx::zenohc::lib)
   ```
   ```
   target_link_libraries(yourproject PUBLIC zenohcxx::zenohc::static)
   ```

- include the [zenoh.hxx] header and use namespace `zenoh`. Depending on preprocessor setting in library target the correct namespace (`zenohpico` or `zenohc`) is aliased to `zenoh` namespace. This allows to write code compatible with both libraries without changes.
    ```C++
    #include "zenoh.hxx"
    using namespace zenoh;
    ```
    or use headers [zenohc.hxx] or [zenohpico.hxx] directly 
    ```C++
    #include "zenohc.hxx"
    using namespace zenohc;
    ```
    ```C++
    #include "zenohpico.hxx"
    using namespace zenohpico;
    ```

## Library API

### Documentation

The library API is not documented yet, but the [api.hxx] file contains commented class definitions and can be used for reference.

### Conventions

There are three main kinds of wrapper classes / structures provided by [zenoh-cpp]. They are:

* Structs derived from `Copyable` template:

```C++
struct PutOptions : public Copyable<::z_put_options_t> {
    ...
}
```

These structures can be freely passed by value. They exacly matches corresponging C-library structures (`z_put_options_t` in this case) and adds some necessary constructors and methods. For example `PutOptions` default constructor calls the zenohc/zenohpico function `z_put_options_default()`.

There are some copyable structures with `View` postfix:

```C++
struct BytesView : public Copyable<::z_bytes_t> {
...
}
```

Such structures contains pointers to some external data. So they should be cared with caution, as they becoming invalid when their data source is destroyed. The same carefulness is required when handling `std::string_view` from standard library for examlple.

* Classes derived from `Owned` template

```C++
class Config : public Owned<::z_owned_config_t> {
...
}
```

Classes which protects corresponding so-called `owned` structures from copying, allowing move semantic only. Corresponding utility functions like `z_check`, `z_null`, `z_drop` are integrated into the `Owned` base template.

* Closures

It's classes representing callback structures:
```C++
typedef ClosureMoveParam<::z_owned_closure_reply_t, ::z_owned_reply_t, Reply> ClosureReply;
typedef ClosureConstPtrParam<::z_owned_closure_query_t, ::z_query_t, Query> ClosureQuery;
```

They allows to wrap C++ invocable objects (fuctions, lambdas, classes with operator() overloaded) and pass them as callbacks to zenoh.


[zenoh-c]: https://github.com/eclipse-zenoh/zenoh-c
[zenoh-cpp]: https://github.com/eclipse-zenoh/zenoh-cpp
[zenoh-pico]: https://github.com/eclipse-zenoh/zenoh-pico
[zenoh.hxx]: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/include/zenoh.hxx 
[zenohc.hxx]: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/include/zenohc.hxx 
[zenohpico.hxx]: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/include/zenohpico.hxx 
[api.hxx]: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/include/zenohcxx/api.hxx 
[add_subdirectory]: https://cmake.org/cmake/help/latest/command/add_subdirectory.html
[find_package]: https://cmake.org/cmake/help/latest/command/find_package.html
[FetchContent]: https://cmake.org/cmake/help/latest/module/FetchContent.html

