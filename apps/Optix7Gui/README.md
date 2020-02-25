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

OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
    -B . ..

cmake --build build --target optix7Gui --parallel 7

./bin/optix7Gui || \
  LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./bin/optix7Gui

LD_PRELOAD=$LD_PRELOAD:/lib/x86_64-linux-gnu/libGL.so \
   bin/optix7Gui

```

Top-level build

```
# navigate to top-level

cd tweeker_raytracer
OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
    -B build .

cmake --build build --target optix7Gui --parallel 7

build/apps/Optix7Gui/bin/optix7Gui || \
  LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu build/apps/Optix7Gui/bin/optix7Gui

```

### Debug build

```
# navigate to top-level of this repo, then:
cd apps/Optix7Gui
mkdir build
cd build

# run cmake generate (runs conan inside)
OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug \
    -B . ..

cmake --build . --target optix7Gui --parallel 7

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
