<img src="https://raw.githubusercontent.com/eclipse-zenoh/zenoh/master/zenoh-dragon.png" height="150">

[![CI](https://github.com/eclipse-zenoh/zenoh-cpp/workflows/CI/badge.svg)](https://github.com/eclipse-zenoh/zenoh-cpp/actions?query=workflow%3A%22CI%22)
[![Documentation Status](https://readthedocs.org/projects/zenoh-cpp/badge/?version=latest)](https://zenoh-cpp.readthedocs.io/en/latest/?badge=latest)
[![Discussion](https://img.shields.io/badge/discussion-on%20github-blue)](https://github.com/eclipse-zenoh/roadmap/discussions)
[![Discord](https://img.shields.io/badge/chat-on%20discord-blue)](https://discord.gg/2GJ958VuHs)
[![License](https://img.shields.io/badge/License-EPL%202.0-blue)](https://choosealicense.com/licenses/epl-2.0/)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# Eclipse zenoh C++ Client API

[Eclipse zenoh](http://zenoh.io) is an extremely efficient and fault-tolerant [Named Data Networking](http://named-data.net) (NDN) protocol 
that is able to scale down to extremely constrainded devices and networks. 

Check the website [zenoh.io](http://zenoh.io) and the [roadmap](https://github.com/eclipse-zenoh/roadmap) for more detailed information.

-------------------------------

## How to build and install it 

The zenoh C++ API is just a set of C++ header wiles wrapping the [zenoh-c] (and [zenoh-pico] in nearest future) library. So to install and use the zenoh-cpp the
[zenoh-c] should be installed also. Though this is not the case for running tests and examples in zenoh-cpp: the CMake build script downloads [zenoh-c] into build
directory for these purposes.

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

2. Clone the [zenoh-cpp] with `git`:

    ```bash
    git clone https://github.com/eclipse-zenoh/zenoh-cpp.git
    cd zenoh-cpp
    ```

3. Install:

* Install [zenoh-c] library as described in it's readme

* Install [zenoh-cpp] headers:

    ```bash
    cd /path/to/zenoh-cpp
    mkdir -p build && cd build 
    cmake .. -DCMAKE_BUILD_TYPE=Headers # configure
    cmake --build . --target install # install - run with `sudo` if necessary
    ```

    Option `CMAKE_BUILD_TYPE=Headers` excludes slow step of downloading and building [zenoh-c] which is required for building 
    tests and examples only and not needed to do install.

    To install headers into path other than `/usr/local` add `-DCMAKE_INSTALL_PREFIX=/destination/path` option on cmake
    onfigure step.

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

These structures can be freely passed by value. They exacly matches corresponging [zenoh-c] structures (`z_put_options_t` in this case)
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

Classes which protects corresponding [zenoh-c] so-called `owned` structures from copying, allowing move semantic only. Corresponding utility functions like `z_check`, `z_null`, `z_drop` are integrated into the `Owned` base template.

* Closures

It's classes representing [zenoh-c] callback structures:
```C++
typedef ClosureMoveParam<::z_owned_closure_reply_t, ::z_owned_reply_t, Reply> ClosureReply;
typedef ClosureConstPtrParam<::z_owned_closure_query_t, ::z_query_t, Query> ClosureQuery;
```

They allows to wrap C++ invocable objects (fuctions, lambdas, classes with operator() overloaded) and pass them as callbacks to zenoh.
