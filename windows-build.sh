sh -c "mkdir -p buildw; cd buildw; mingw64 cmake -DCMAKE_TOOLCHAIN_FILE=/home/dev/dc/simulant-engine/toolchains/Mingw.cmake -DCMAKE_BUILD_TYPE=Debug -DHOST=x86_64-w64-mingw32 .. && mingw64 make -j5"
