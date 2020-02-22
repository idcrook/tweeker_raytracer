tweeker_raytracer
=================

Experimental raytracing test bed.

![intro Optix 10 - denoiser and env](assets/img/intro_10_altenv_light_1024pp.png)

Applications:

-	[OptixGui](src/OptixGui) - based directly on https://github.com/nvpro-samples/optix_advanced_samples/tree/master/src/optixIntroduction

Name is a play on my earlier project `weeker_raytracer`. This repo is spun out of [weeker_raytracer](https://github.com/idcrook/weeker_raytracer), which itself was based on Ray Tracing In One Weekend series by Peter Shirley.

Pre-requisites
--------------

Tested on Ubuntu Linux 19.10 on an amd64 system with RTX GeForce card.

-	Uses CMake to build.
-	Uses [conan](https://conan.io/) for some C++ library package management
-	Requires CUDA SDK and tools and Optix SDK(s) to be installed locally to build/link.

Refer to `notes` directory contents, including [optix install](notes/optix/install.md)

Uses conan as C++ dependency manager. In general this means

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo
3.	Install dependencies (See respective README)
	-	Generate dependency package cmake includes (via `conan`\)
	-	Use CMake to generate Makefiles
	-	Use cmake to compile/build project sources and libs
4.	Run

### Other Third-party dependencies

#### Install DevIL image library

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

OptiX 6.5 gui.

-	Uses imgui (using GLFW and OpenGL3 via GLEW)
	-	These are installed using conan.io
-	Uses DevIL image library
-	Assumes [optix_advanced_samples](https://github.com/nvpro-samples/optix_advanced_samples) repo is available for its texture files

See respective [README.md](src/OptixGui/README.md) for build instructions and `conanfile.txt` for dependencies.

`optix7Gui`
-----------

-	Uses imgui (using GLFW and OpenGL3 via GLEW)
	-	These are installed using conan.io
-	Uses DevIL image library
-	Assumes [OptiX_Apps](https://github.com/NVIDIA/OptiX_Apps) repo is available for its texture files

See respective [README.md](src/Optix7Gui/README.md) for build instructions and `conanfile.txt` for dependencies.

Build `optixGui`
----------------

Refer to [OptixGui README](src/OptixGui/README.md)

### Run

After a successful build

```shell
cd tweeker_raytracer # top-level directory again
# run
build/optixGui [options]
```

Image renders and screencaps from `optixGui`
--------------------------------------------

![intro Optix 04 - shapes](assets/img/intro_04.png)

![intro Optix 06 - alternative camera projections nested materials](assets/img/intro_06.png) Light, nested materials

![intro Optix 07 - image textures including environment map](assets/img/intro_07.png) image textures including environment map

![intro Optix 09 - denoiser off](assets/img/intro_09_denoise_off_8pp.png) denoiser OFF, 8 samples per pixel

![intro Optix 09 - denoiser on](assets/img/intro_09_denoise_on_8pp.png) denoiser ON, 8 samples per pixel

![intro Optix 10 - denoiser and env](assets/img/intro_10_altenv_light_1024pp.png) Alternate environment, light, 1k samples per pixel
