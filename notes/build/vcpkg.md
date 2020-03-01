Install
=======

git clone and bootstrap

```shell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
```

output

```
❯ ./vcpkg integrate install
Applied user-wide integration for this vcpkg root.

CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=/home/dpc/projects/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

example install
---------------

```shell
cd ~/projects/vcpkg
./vcpkg install assimp
```

output

```
 Computing installation plan...
The following packages are already installed:
    assimp[core]:x64-linux
Package assimp:x64-linux is already installed

Total elapsed time: 8.66 us

The package assimp:x64-linux provides CMake targets:

    find_package(assimp CONFIG REQUIRED)
    target_link_libraries(main PRIVATE assimp::assimp)

```

similarly for imgui

```
    find_package(imgui CONFIG REQUIRED)
    target_link_libraries(main PRIVATE imgui::imgui)

```

using in this project
---------------------

```
OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug    \
    -DCMAKE_TOOLCHAIN_FILE=${HOME}/projects/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -B . ..

```
