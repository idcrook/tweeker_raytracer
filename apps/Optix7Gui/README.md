Using conan as C++ dependency manager
=====================================

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo:
3.	Install dependencies (via `conan`), compile and run

Assumes DevIL headers and library are installed on system.

Notes
-----

Since am using more recent imgui version compared to the original sources, adjusted the calls into the imgui bindings and updated the styles.

### Point to samples directory

Expects textures from this repository: https://github.com/NVIDIA/OptiX_Apps/tree/master/data

Can use symlink `data` directory

```
cd apps/Optix7Gui
ln -s /path/to/OptiX_Apps/data data
```

Linux
-----

Any build will require pointing to SDK. Not yet working from top-level CMake.

```bash
# navigate to top-level of this repo, then:
cd apps/Optix7Gui
mkdir build
cd build

# workaround for busted main repo libx11
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan | true

conan install .. -s build_type=Release

#cmake .. -DCMAKE_BUILD_TYPE=Release
OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
    -B . ..

OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
    -B build apps

conan remote remove bincrafters

cmake --build . --target optix7Gui --parallel 7
cmake --build build --target optix7Gui --parallel 7

./bin/optix7Gui || \
  LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./bin/optix7Gui

OPTIX7_LOCAL_PTX_DIR=`pwd`/build/Optix7Gui/bin/optix7gui_core \
build/Optix7Gui/bin/optix7Gui || \
  OPTIX7_LOCAL_PTX_DIR=`pwd`/build/Optix7Gui/bin/optix7gui_core \
  LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu build/Optix7Gui/bin/optix7Gui


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

./bin/optix7Gui || \
  LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./bin/optix7Gui

```

setting up in `cuda-gdb`

```shell
cd /home/dpc/projects/learning/rt/tweeker_raytracer/apps/Optix7Gui/build
file bin/optix7Gui
set env LD_LIBRARY_PATH /usr/lib/x86_64-linux-gnu
run
```

using the `cuda-gdb` for `gdb` in emacs

```lisp
(setq gud-gdb-command-name "cuda-gdb -i=mi --args ")
```
