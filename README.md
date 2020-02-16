tweeker_raytracer
=================

Experimental raytracing test bed. Spun out of [weeker_raytracer](https://github.com/idcrook/weeker_raytracer) which was based on Ray Tracing In One Weekend series by Peter Shirley.

Pre-requisites
--------------

-	Uses CMake
-	Uses [conan](https://conan.io/) for C++ package management (Experimental)

Using conan as C++ dependency manager

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo:
3.	Install dependencies (via `conan`), cmake generete, compile and run

See `conanfile.txt` for dependencies.

### install devil

http://openil.sourceforge.net/

```shell
sudo apt install libdevil-dev
```

Installing `-dev` also installs the library package.

Build `optixGui`
----------------

Tested in Ubuntu Linux. Start with Debug build

```shell
# navigate to repo clone
cd tweeker_raytracer
cd src/OptixGui
mkdir build
cd build

# workaround for busted main repo libX11 dependency
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan | true

conan install .. -s build_type=Debug

# remove workaround remote if desired
conan remote remove bincrafters


# now run CMake at top-level
cd ../../../../tweeker_raytracer/
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug \
 -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
 -B build src

cmake --build build --target optixGui --parallel 7

build/optixGui # [options]

```
