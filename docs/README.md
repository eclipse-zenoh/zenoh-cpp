# Building the documentation

## Install tools

- Ubuntu

  ```bash
  sudo apt install doxygen
  ```

- MacOS

  ```bash
  brew install doxygen
  ```

```bash
pip3 install sphinx
pip3 install breathe
pip3 install sphinx-rtd-theme
```

## Build documentation locally

```bash
git clone https://github.com/eclipse-zenoh/zenoh-cpp.git
mkdir build
cmake -Szenoh-cpp -Bbuild
```

the output should be

```raw
...
-- Found Doxygen: /usr/bin/doxygen (found version "1.9.1") found components: doxygen missing components: dot
-- Found Sphinx: /home/username/.local/bin/sphinx-build  
...
```

or something like this. If doxygen or sphinx are not found, check first step. If everything is ok,
build the documentation with this command:

```bash
cmake --build build --target docs
```

If everything is ok, the output is

```raw
...
See read-the-docs html in /username/build/docs/docs/sphinx/index.html
...
```
