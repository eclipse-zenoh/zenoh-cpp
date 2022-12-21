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

This repository provides a C++ binding based on the [Zenoh C API](https://github.com/eclipse-zenoh/zenoh-c).

C++ bindings are still so the Zenoh team will highly appreciate any help in testing them on various platforms, system architecture, etc. and to report any issue you might encounter. This will help in greatly improving its maturity and robustness.

-------------------------------
## How to build and install it 

> :warning: **WARNING** :warning: : Zenoh and its ecosystem are under active development. When you build from git, make sure you also build from git any other Zenoh repository you plan to use (e.g. binding, plugin, backend, etc.). It may happen that some changes in git are not compatible with the most recent packaged Zenoh release (e.g. deb, docker, pip). We put particular effort in mantaining compatibility between the various git repositories in the Zenoh project.

The zenoh C++ API is a set of C++ header files wrapping the [zenoh-c](https://github.com/eclipse-zenoh/zenoh-c) (and [zenoh-pico(https://github.com/eclipse-zenoh/zenoh-pico)] in nearest future) library. 
The zenoh C++ API is a set of C++ header files wrapping the [zenoh-c](https://github.com/eclipse-zenoh/zenoh-c) (and [zenoh-pico] in nearest future) library. 
So to install and use zenoh-cpp, [zenoh-c](https://github.com/eclipse-zenoh/zenoh-c) should be installed in the system. 

[zenoh-c]: https://github.com/eclipse-zenoh/zenoh-c
[zenoh-cpp]: https://github.com/eclipse-zenoh/zenoh-cpp
[zenoh-pico]: https://github.com/eclipse-zenoh/zenoh-pico
[zenohcpp.h]: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/include/zenohcpp.h 

The steps to install [zenoh-cpp]:

1. Make sure that [rust](https://www.rust-lang.org) is available on your platform:
    
* Ubuntu
    ```bash
    sudo apt-get install rustc
    ```

* MacOS
    ```bash
    brew install rust
    ```

2. Install [zenoh-c](https://github.com/eclipse-zenoh/zenoh-c) library. If you don't want to use root privileges and install it into system `/usr/local` directory 
add [CMAKE_INSTALL_PREFIX](https://cmake.org/cmake/help/v3.0/variable/CMAKE_INSTALL_PREFIX.html) parameter to `cmake` arguments.

    ```sh
    git clone https://github.com/eclipse-zenoh/zenoh-c.git &&
    cd zenoh-c && mkdir -p build && cd build &&
    cmake .. -DCMAKE_INSTALL_PREFIX=/home/username/local &&
    cmake --build . --target install
    ```

3. Install [zenoh-cpp]. Use the same `CMAKE_INSTALL_PREFIX` parameter as for [zenoh-c](https://github.com/eclipse-zenoh/zenoh-c).  The key point is that this
parameter is not only install destination path, but is also included into `CMAKE_SYSTEM_PREFIX_PATH`. So the CMAKE's 
[find_package](https://cmake.org/cmake/help/latest/command/find_package.html) command is able to find [zenoh-c].
The path must be absolute, without environment variables and home directory shortcut `~`, otherwise cmake may not be able to
find it. 

    ```sh
    git clone https://github.com/eclipse-zenoh/zenoh-cpp.git &&
    cd zenoh-cpp && mkdir -p build && cd build &&
    cmake .. -DCMAKE_INSTALL_PREFIX=/home/username/local &&
    cmake --build . --target install
    ```

## Building and running tests

```bash
cd /path/to/zenoh-cpp
mkdir -p build && cd build 
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target tests
ctest
```

## Building the Examples

```bash
cd /path/to/zenoh-cpp
mkdir -p build && cd build 
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target examples
```

You may alternatively use other CMAKE_BUILD_TYPE configurations, such as `Debug` or `RelWithDebInfo` if you wish to keep the debug symbols.

## Running the Examples

### Basic Pub/Sub Example
```bash
./target/release/examples/z_sub_cpp
```

```bash
./target/release/examples/z_pub_cpp
```

### Queryable and Query Example
```bash
./target/release/examples/z_queryable_cpp
```

```bash
./target/release/examples/z_get_cpp
```

### Throughput Examples
```bash
./target/release/examples/z_sub_thgr_cpp
```

```bash
./target/release/examples/z_pub_thgr_cpp
```

## Library usage and API Conventions

To use the library include the [zenohcpp.h] header. Other header files are not guaranteed to keep their names in future.
```C++
#include "zenohcpp.h"
using namespace zenoh;
```

There are three main kinds of wrapper classes / structures provided by [zenoh-cpp]. They are:

* Structs derived from `Copyable` template:

```C++
struct PutOptions : public Copyable<::z_put_options_t> {
    ...
}
```

These structures can be freely passed by value. They exacly matches corresponging [zenoh-c](https://github.com/eclipse-zenoh/zenoh-c) structures (`z_put_options_t` in this case)
and adds some necessary constructors and methods. For example `PutOptions` default constructor calls the zenoh function
`z_put_options_default()`.

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

Classes which protects corresponding [zenoh-c](https://github.com/eclipse-zenoh/zenoh-c) so-called `owned` structures from copying, allowing move semantic only. Corresponding utility functions like `z_check`, `z_null`, `z_drop` are integrated into the `Owned` base template.

* Closures

It's classes representing [zenoh-c](https://github.com/eclipse-zenoh/zenoh-c) callback structures:
```C++
typedef ClosureMoveParam<::z_owned_closure_reply_t, ::z_owned_reply_t, Reply> ClosureReply;
typedef ClosureConstPtrParam<::z_owned_closure_query_t, ::z_query_t, Query> ClosureQuery;
```

They allows to wrap C++ invocable objects (fuctions, lambdas, classes with operator() overloaded) and pass them as callbacks to zenoh.
