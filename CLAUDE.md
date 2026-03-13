## Build project

1) Install zenoh-c and zenoh-pico to ~/local with unstable api and shared memory enabled in debug variant

./scripts/install_from_git.sh ~/local TRUE TRUE TRUE Debug

2) Build tests and examples

mkdir build
cd build
cmake .. -DZENOHCXX_ZENOHC=ON -DZENOHCXX_ZENOHPICO=ON -DCMAKE_INSTALL_PREFIX=~/local -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target tests
cmake --build . --target examples
