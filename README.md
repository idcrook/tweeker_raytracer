tweeker_raytracer
=================

Experimental raytracing test bed.

![intro Optix 10 - denoiser and env](assets/img/intro_10_altenv_light_1024pp.png)

Applications:

-	[OptixGui](src/OptixGui) - based directly on https://github.com/nvpro-samples/optix_advanced_samples/tree/master/src/optixIntroduction

Name is a play on my earlier project `weeker_raytracer`. This repo is spun out of [weeker_raytracer](https://github.com/idcrook/weeker_raytracer), which itself was based on Ray Tracing In One Weekend series by Peter Shirley.

Pre-requisites
--------------

Tested on Ubuntu Linux 19.10.

-	Uses CMake to build.
-	Uses [conan](https://conan.io/) for some C++ library package management
-	Requires CUDA SDK and tools and Optix SDK to be installed locally to build/link. Refer to `notes` directory, including [optix install](notes/optix/install.md)
-	Requires [optix_advanced_samples](https://github.com/nvpro-samples/optix_advanced_samples) repo for its texture files

Uses conan as C++ dependency manager

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo:
3.	Install dependencies (See below)
	-	Generate cmake package includes (via `conan`\)
	-	Use CMake to generate
	-	Use cmake to compile/build project sources and libs
4.	Run

See `conanfile.txt` for dependencies.

### Install devil image library

Dependency not in `conan` but should be available via package manager.

```shell
sudo apt install libdevil-dev
```

Installing `-dev` also installs the library package.

http://openil.sourceforge.net/

Build `optixGui`
----------------

Start with Debug build

```shell
# navigate to repo clone
cd tweeker_raytracer
cd src/OptixGui
mkdir build
cd build

# workaround for broken (as of 15-Feb-2020) libX11 dependency in conan's main remote
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan | true

# run conan which will download libraries, and create cmake module includes
conan install .. -s build_type=Debug

# remove workaround remote, if desired
conan remote remove bincrafters

# Now, run CMake generate stage at top-level
cd ../../../../tweeker_raytracer/
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug \
 -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
 -B build src

# Build binary target
cmake --build build --target optixGui --parallel 7
```

### Point to Samples Directory

Relies upon directory from this repository: https://github.com/nvpro-samples/optix_advanced_samples/tree/master/src/data

Required for images texture files used. Otherwise, program will segfault upon startup.

Can use an envariable `OPTIX_SAMPLES_SDK_DIR` to point to a clone, or even symlink `data` directory

```shell
export OPTIX_SAMPLES_SDK_DIR=/path/to/optix_advanced_samples/src

# or, in a clone of this repo, symlink to the data directory inside project directory

cd src/OptixGui
ln -s /path/to/optix_advanced_samples/src/data data
```

Run
---

```shell
cd tweeker_raytracer # top-level directory again
# run
build/optixGui [options]
```

try with Releaase build
-----------------------

```shell
# navigate to repo clone
cd tweeker_raytracer
cd src/OptixGui
mkdir build
cd build

# workaround for broken (as of 15-Feb-2020) libX11 dependency in conan's main remote
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan | true

# run conan which will download libraries, and create cmake module includes
conan install .. -s build_type=Release

# remove workaround remote, if desired
conan remote remove bincrafters

# Now, run CMake generate stage at top-level
cd ../../../../tweeker_raytracer/
OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 cmake \
 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
 -B build src

# Build binary target
cmake --build build --target optixGui --parallel 7
```

Image renders and screencaps from `optixGui`
--------------------------------------------

![intro Optix 04 - shapes](assets/img/intro_04.png)

![intro Optix 06 - alternative camera projections nested materials](assets/img/intro_06.png) Light, nested materials

![intro Optix 07 - image textures including environment map](assets/img/intro_07.png) image textures including environment map

![intro Optix 09 - denoiser off](assets/img/intro_09_denoise_off_8pp.png) denoiser OFF, 8 samples per pixel

![intro Optix 09 - denoiser on](assets/img/intro_09_denoise_on_8pp.png) denoiser ON, 8 samples per pixel

![intro Optix 10 - denoiser and env](assets/img/intro_10_altenv_light_1024pp.png) Alternate environment, light, 1k samples per pixel
