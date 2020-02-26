Using conan as C++ dependency manager
=====================================

1.	Install conan: https://docs.conan.io/en/latest/installation.html
2.	Clone this repo:
3.	Install dependencies (via `conan`), compile and run

Assumes headers and library are installed on system:

-	DevIL
-	GLEW
-	glfw

Notes
-----

Since am using more recent imgui version compared to the original sources, adjusted the calls into the imgui bindings and updated the styles.

### Point to samples directory

Expects textures from this repository: https://github.com/NVIDIA/OptiX_Apps/tree/master/data

Can use symlink `data` directory

```
cd apps/rtigo3
ln -s /path/to/OptiX_Apps/data data
```

Linux
-----

Any build will require pointing to SDK.

```bash
# navigate to top-level of this repo, then:
cd apps/rtigo3
mkdir build
cd build

OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 \
cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
    -B . ..

cmake --build build --target rtigo3 --parallel 7

cd ../data/

../build/bin/rtigo3 \
  -s ./system_rtigo3_cornell_box.txt -d ./scene_rtigo3_cornell_box.txt

```

Top-level build

```
# navigate to top-level

cd tweeker_raytracer
OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64
cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
    -B build .

cmake --build build --target rtigo3 --parallel 7

cd apps/rtigo3/data
../../build/apps/rtigo3/bin/rtigo3 \
  -s ./system_rtigo3_cornell_box.txt -d ./scene_rtigo3_cornell_box.txt
```

### Debug build

```
# navigate to top-level of this repo, then:
cd apps/rtigo3
mkdir build
cd build

# run cmake generate (runs conan inside)
OPTIX7_PATH=/usr/local/nvidia/NVIDIA-OptiX-SDK-7.0.0-linux64 \
cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug \
    -B . ..

cmake --build . --target rtigo3 --parallel 7

../build/bin/rtigo3 \
  -s ./system_rtigo3_cornell_box.txt -d ./scene_rtigo3_cornell_box.txt

```

setting up in `cuda-gdb`

```shell
cd /home/dpc/projects/learning/rt/tweeker_raytracer/apps/rtigo3/build
file bin/rtigo3

run -s /home/dpc/projects/learning/rt/github/optix/OptiX_Apps/data/system_rtigo3_cornell_box.txt -d /home/dpc/projects/learning/rt/github/optix/OptiX_Apps/data/scene_rtigo3_cornell_box.txt
```

using the `cuda-gdb` for `gdb` in emacs

```lisp
(setq gud-gdb-command-name "cuda-gdb -i=mi  ")
```

also

```shell
cd ~/projects/learning/rt/tweeker_raytracer/apps/rtigo3/data

~/projects/learning/rt/tweeker_raytracer/apps/rtigo3/build/bin/rtigo3 \
  -s ./system_rtigo3_cornell_box.txt -d ./scene_rtigo3_cornell_box.txt
```
