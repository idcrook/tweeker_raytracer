Using conan as C++ dependency manager
=====================================

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo:
3.	Install dependencies (via `conan`), compile and run

Assumes headers and library are installed on system:

-	DevIL
-	GLEW
-	glfw

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

OptiX_INSTALL_DIR="/usr/local/nvidia/NVIDIA-OptiX-SDK-6.5.0-linux64/" \
cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
    -B . ..

cmake --build . --target optixGui --parallel 7

bin/optixGui
```

### Debug build

```bash
# navigate to top-level of this clone, then:
cd apps/OptixGui
mkdir build
cd build

OptiX_INSTALL_DIR="/usr/local/nvidia/NVIDIA-OptiX-SDK-6.5.0-linux64/" \
cmake \
  -D CMAKE_BUILD_TYPE=Debug \
  -D CMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -D CMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
  -B . ..

cmake --build . --target optixGui --parallel 7

bin/optixGui
```

swap `-DCMAKE_BUILD_TYPE=Release` from `-DCMAKE_BUILD_TYPE=Debug` in the above commands to get Release version of dependencies, etc.

### setting up in cuda-gdb

```shell
cd /home/dpc/projects/learning/rt/tweeker_feature/apps/OptixGui/build
file optixGui
# do not need anymore # set env LD_LIBRARY_PATH /usr/lib/x86_64-linux-gnu
run
```
