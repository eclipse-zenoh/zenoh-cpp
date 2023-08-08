# Building the documentation

### Install tools

-   Ubuntu
    ```
    sudo apt install doxygen
    ```

-   MacOS
    ```
    brew install doxygen
    ```

```
pip3 install sphinx
pip3 install breathe
pip3 install sphinx-rtd-theme
```

### Build documentation locally

```
git clone https://github.com/eclipse-zenoh/zenoh-cpp.git
mkdir build
cmake -Szenoh-cpp -Bbuild
```

the output should be 
```
...
-- Found Doxygen: /usr/bin/doxygen (found version "1.9.1") found components: doxygen missing components: dot
-- Found Sphinx: /home/username/.local/bin/sphinx-build  
...
```
or something like this. If doxygen or sphinx are not found, check first step. If everything is ok,
build the documentation with this command:

```
cmake --build build --target docs
```

If everrything is ok, the output is
```
...
See read-the-docs html in /username/build/docs/docs/sphinx/index.html
...
```
