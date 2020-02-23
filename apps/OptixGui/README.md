Using conan as C++ dependency manager
=====================================

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo:
3.	Install dependencies (via `conan`), compile and run

Assumes DevIL headers and library are installed on system.

### Point to samples directory

Relies upon directory from this repository: https://github.com/nvpro-samples/optix_advanced_samples/tree/master/src/data

Required for images texture files used. Otherwise, program will segfault upon startup.

Can use an envariable `OPTIX_SAMPLES_SDK_DIR` to point to a clone, or even symlink `data` directory

```shell
export OPTIX_SAMPLES_SDK_DIR=/path/to/optix_advanced_samples/src

# or, in a clone of this repo, symlink to the data directory inside project directory

cd apps/OptixGui
ln -s /path/to/optix_advanced_samples/src/data data
```

Linux
-----

Any build will require pointing to Optix 6.5 SDK.

```bash
# navigate to top-level of this clone, then:
cd apps/OptixGui
mkdir build
cd build

# workaround for busted main repo libx11
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan | true

conan install .. -s build_type=Debug
conan install .. -s build_type=Release

# if you are only interested in this app from the repo
#cmake .. -DCMAKE_BUILD_TYPE=Release
cmake \
    -DOptiX_INSTALL_DIR="/usr/local/nvidia/NVIDIA-OptiX-SDK-6.5.0-linux64/" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
    -B . ..

conan remote remove bincrafters

cmake --build . --target optixGui --parallel 7

./optixGui
```

Once you've gathered the dependencies using conan, you can also switch to running CMake at the top-level. so starting above example at the CMake generate step

```shell
# navigate to repo clone toplevel
cd tweeker_raytracer

# Now, run CMake generate stage at top-level
# Assumes you have already run conan in each of the lower directories
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
 -B build apps

# Build binary target
cmake --build build --target optixGui --parallel 7

build/optixGui
```

Note that builds using `build_type=Release` (`conan`) and `-D CMAKE_BUILD_TYPE=Release`, the respective types should match.

### Debug build

```
# in case you have not run it yet
conan install .. -s build_type=Debug

# run generate
# if you are only interested in this app from the repo
cmake \
    -DOptiX_INSTALL_DIR="/usr/local/nvidia/NVIDIA-OptiX-SDK-6.5.0-linux64/" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
    -B . ..

cmake --build . --target optixGui --parallel 7

./optixGui

```

setting up in cuda-gdb

```shell
cd /home/dpc/projects/learning/rt/tweeker_feature/apps/OptixGui/build
file optixGui
# do not need anymore # set env LD_LIBRARY_PATH /usr/lib/x86_64-linux-gnu
run
```
