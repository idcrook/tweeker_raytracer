Using conan as C++ dependency manager
=====================================

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo:
3.	Install dependencies (via `conan`), compile and run

Assumes DevIL headers and library are installed on system.

### Point to samples directory

Expects textures from this repository: https://github.com/NVIDIA/OptiX_Apps/tree/master/data

Can use symlink `data` directory

```
cd src/Optix7Gui
ln -s /path/to/OptiX_Apps/data data
```

Linux
-----

Any build will require pointing to SDK. Not yet working from top-level CMake.

```bash
# navigate to top-level of this repo, then:
cd src/Optix7Gui
mkdir build
cd build

# workaround for busted main repo libx11
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan | true

conan install .. -s build_type=Release

#cmake .. -DCMAKE_BUILD_TYPE=Release
OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
    -B . ..

conan remote remove bincrafters

cmake --build . --target optix7Gui --parallel 7

LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./optix7Gui
./optix7Gui

```

### Debug build

```
conan install .. -s build_type=Debug
# run generate
OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug \
    -B . ..

cmake --build . --target optix7Gui --parallel 7

# LD_LIBRARY_PATH needed so that system Nvidia opengl drivers are used
LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./optix7Gui

```

setting up in cuda-gdb

```shell
cd /home/dpc/projects/learning/rt/tweeker_raytracer/src/Optix7Gui/build
file optix7Gui
set env LD_LIBRARY_PATH /usr/lib/x86_64-linux-gnu
run
```
