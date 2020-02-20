Using conan as C++ dependency manager
=====================================

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo:
3.	Install dependencies (via `conan`), compile and run

### install devil

http://openil.sourceforge.net/

installing `dev` also installs the lib

```
sudo apt install libdevil-dev
```

### Point to samples directory

Relies upon directory from this repository: https://github.com/nvpro-samples/optix_advanced_samples/tree/master/src/data

Can use envariable `OPTIX_SAMPLES_SDK_DIR` to point to a clone, or even symlink `data` directory

```
export OPTIX_SAMPLES_SDK_DIR=/path/to/optix_advanced_samples/src

# or, in a clone of this repo, symlink to the data directory inside project directory

cd src/OptixGui
ln -s /path/to/optix_advanced_samples/src/data data
```

Linux
-----

Any build will require pointing to SDK.

```bash
# navigate to top-level of this repo, then:
cd src/OptixGui
mkdir build
cd build

# workaround for busted main repo libx11
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan | true

conan install .. -s build_type=Release
#cmake .. -DCMAKE_BUILD_TYPE=Release
cmake \
    -DOptiX_INSTALL_DIR="/usr/local/nvidia/NVIDIA-OptiX-SDK-6.5.0-linux64/" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
    -B . ..

conan remote remove bincrafters

cmake --build . --target optixGui --parallel 7

# do not need anymore # LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./optixGui
./optixGui

```

### Debug build

```
conan install .. -s build_type=Debug
# run generate
cmake \
    -DOptiX_INSTALL_DIR="/usr/local/nvidia/NVIDIA-OptiX-SDK-6.5.0-linux64/" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
    -B . ..

cmake --build . --target optixGui --parallel 7

# do not need anymore # LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./optixGui
./optixGui

```

setting up in cuda-gdb

```shell
cd /home/dpc/projects/learning/rt/tweeker_feature/src/OptixGui/build
file optixGui
#  do not need anymore # set env LD_LIBRARY_PATH /usr/lib/x86_64-linux-gnu
run
```
