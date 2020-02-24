tweeker_raytracer
=================

Experimental raytracing test bed.

![OptiX 7 - scene](assets/img/optix7_running.png)

Name is a play on my earlier project `weeker_raytracer`. This repo is spun out of [weeker_raytracer](https://github.com/idcrook/weeker_raytracer), which itself was based on Ray Tracing In One Weekend series by Peter Shirley.

Pre-requisites
--------------

Tested on Ubuntu Linux 19.10 on an x86-64 system with RTX GeForce card.

-	Uses CMake to build.
-	Uses [conan](https://conan.io/) for some C++ library package management
-	Requires CUDA SDK and tools and Optix SDK(s) to be installed locally to build/link.

Refer to `notes` directory contents, including [optix install](notes/optix/install.md)

Uses conan as C++ dependency manager. In general this means

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo
3.	Install dependencies using conan or system package manager
	-	Generate dependency package cmake includes (via `conan`\)
	-	Use CMake to generate Makefiles
	-	Use cmake to compile/build project sources and libs
4.	Run

### Other Third-party dependencies

In addition to baseline requirements of CUDA SDK and Optix 6.5 and/or Optix 7.0 SDK (refer to `notes` directory for how I did it) there are addition libraries to links into the apps.

#### Install DevIL developer image library

http://openil.sourceforge.net/

Dependency not available in `conan` but should be available via system package manager.

```shell
sudo apt install libdevil-dev
```

Installing `-dev` package also brings in the library package.

Apps
----

`optixGui`
----------

-	[OptixGui](apps/OptixGui) - based directly on https://github.com/nvpro-samples/optix_advanced_samples/tree/master/src/optixIntroduction

OptiX 6.5 imgui

-	Uses imgui (using GLFW and OpenGL3 via GLEW)
	-	These are installed using conan.io
-	Uses DevIL image library
-	Assumes [optix_advanced_samples](https://github.com/nvpro-samples/optix_advanced_samples) repo is available for its texture files

See respective [README.md](apps/OptixGui/README.md) for build instructions and `conanfile.txt` for dependencies.

`optix7Gui`
-----------

-	[Optix7Gui](apps/Optix7Gui) - based directly on https://github.com/NVIDIA/OptiX_Apps/tree/master/apps/intro_denoiser

OptiX 7 imgui

-	Uses imgui (using GLFW and OpenGL3 via GLEW)
	-	These are installed using conan.io
-	Uses DevIL image library
-	Assumes [OptiX_Apps](https://github.com/NVIDIA/OptiX_Apps) repo is available for its texture files

See respective [README.md](apps/Optix7Gui/README.md) for build instructions and `conanfile.txt` for dependencies.

Build on Linux
--------------

IMPORTANT: Refer to respective readme for pointing to texture files.

-	[OptixGui README](apps/OptixGui/README.md)
-	[Optix7Gui README](apps/Optix7Gui/README.md)

```bash
# navigate to top-level of this repo
cd tweeker_raytracer

OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 \
OptiX_INSTALL_DIR="/usr/local/nvidia/NVIDIA-OptiX-SDK-6.5.0-linux64/" \
cmake \
    -D CMAKE_BUILD_TYPE=Release \
    -B build apps

cmake --build build --parallel 7
```

### Run

After a successful build

```shell
cd tweeker_raytracer # top-level directory again

build/OptixGui/bin/optixGui -h
build/Optix7Gui/bin/optix7Gui -h

# optixGui - FIXME: update path handling
OPTIX_SAMPLES_SDK_PTX_DIR=`pwd`/build/lib/ptx \
    build/OptixGui/bin/optixGui

# optix7Gui - FIXME: top-level build cannot locate texture images
build/Optix7Gui/bin/optix7Gui || \
  LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu build/Optix7Gui/bin/optix7Gui
```

Image renders and screencaps from `optixGui`
--------------------------------------------

![intro Optix 04 - shapes](assets/img/intro_04.png)

![intro Optix 06 - alternative camera projections nested materials](assets/img/intro_06.png) Light, nested materials

![intro Optix 07 - image textures including environment map](assets/img/intro_07.png) image textures including environment map

![intro Optix 09 - denoiser off](assets/img/intro_09_denoise_off_8pp.png) denoiser OFF, 8 samples per pixel

![intro Optix 09 - denoiser on](assets/img/intro_09_denoise_on_8pp.png) denoiser ON, 8 samples per pixel

![intro Optix 10 - denoiser and env](assets/img/intro_10_altenv_light_1024pp.png) Alternate environment, light, 1k samples per pixel

Image renders and screencaps from `optix7Gui`
---------------------------------------------

![OptiX 7 - compare to optix 6](assets/img/optix7_compare.png) optix7Gui, alternate environment, light, running in app.
