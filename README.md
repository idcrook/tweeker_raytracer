tweeker_raytracer
=================

Experimental raytracing test bed.

![intro Optix 06 - alternative camera projections nested materials](assets/img/intro_06.png)

Applications:

-	[OptixGui](src/OptixGui) - based directly on https://github.com/nvpro-samples/optix_advanced_samples/tree/master/src/optixIntroduction

Name is a play on `weeker_raytracer`. This repo is spun out of [weeker_raytracer](https://github.com/idcrook/weeker_raytracer), which itself was based on Ray Tracing In One Weekend series by Peter Shirley.

Pre-requisites
--------------

-	Uses CMake to build.
-	Uses [conan](https://conan.io/) for some C++ library package management
-	Requires CUDA SDK and tools and Optix SDK to be installed locally. Refer to `notes` directory, including [optix install](notes/optix/install.md)

Tested on Ubuntu Linux 19.10.

Uses conan as C++ dependency manager

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo:
3.	Install dependencies (via `conan`), cmake generate, compile and run

See `conanfile.txt` for dependencies.

### install devil

http://openil.sourceforge.net/

```shell
sudo apt install libdevil-dev
```

Installing `-dev` also installs the library package.

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

# remove workaround remote if desired
conan remote remove bincrafters

# now run CMake generate at top-level
cd ../../../../tweeker_raytracer/
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug \
 -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
 -B build src

# build
cmake --build build --target optixGui --parallel 7

# run
build/optixGui # [options]

# run, pointing to the sample files via envariable
export OPTIX_SAMPLES_SDK_DIR=/home/dpc/projects/learning/rt/github/optix/optix_advanced_samples/src

build/optixGui

```

Image renders for `optixGui`
----------------------------

![intro Optix 04 - shapes](assets/img/intro_04.png)

![intro Optix 06 - alternative camera projections nested materials](assets/img/intro_06.png)

![intro Optix 07 - image textures including environment map](assets/img/intro_07.png)
