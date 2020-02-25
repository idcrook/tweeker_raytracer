C++ package manager
===================

https://docs.conan.io/en/latest/introduction.html

https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html

install conan
-------------

```
sudo apt install ~/projects/learning/rt/github/conan/conan-ubuntu-64_1_22_2.deb
# https://github.com/bincrafters/community/issues/1097
# needed to get glfw dependencies
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
```

```
❯ conan search 'x11*' --remote=conan-center


There are no packages matching the 'x11*' pattern

> conan search 'glf*' --remote=conan-center
Existing package recipes:

glfw/3.2.1@bincrafters/stable
glfw/3.2.1.20180327@bincrafters/stable
glfw/3.3@bincrafters/stable
glfw/3.3.1@bincrafters/stable
glfw/3.3.2@bincrafters/stable
```

example demo
------------

download dependencies with conan

build with cmake

```shell
# needed by meson to build mesa
sudo apt install -y python3-mako
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan | true

cd ~/projects/learning/rt/github/conan/examples/libraries/dear-imgui/basic/build
#conan install .. --build=mesa -s build_type=Release
#cmake .. -DCMAKE_BUILD_TYPE=Release
conan install .. --build=mesa -s build_type=Debug
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . --clean-first

conan remote remove bincrafters
```

run

```
./dear-imgui-conan
MESA-LOADER: failed to open swrast (search paths /home/conan/.conan/data/mesa/19.3.1/bincrafters/stable/package/a56cf85a12b68f87c51b8bc2331fe996caedb686/lib/dri)
libGL error: failed to load driver: swrast
Glfw Error 65543: GLX: Failed to create context: BadMatch

strace ./dear-imgui-conan >& log.out
#  the system one works!
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libGL.so.1", O_RDONLY|O_CLOEXEC) = 3
```

problem with the conan mesa `libGL` for my system, so just delete it

```
# https://askubuntu.com/a/903488

sudo ldconfig -p | grep -i gl.so

# had to delete the libGL.so.1 files in

❯  find /home/dpc/.conan -iname "*libGL.so*" -exec ls -l -- {} +


```

dead-ends Debugging failure
---------------------------

install

```
❯ glxinfo -B

Command 'glxinfo' not found, but can be installed with:

sudo apt install mesa-utils

❯ sudo apt install mesa-utils

❯ sudo apt install libgl1-mesa-glx

❯ find /usr -iname "*libGL.so*" -exec ls -l -- {} +

lrwxrwxrwx 1 root root     14 Mar 13  2019 /usr/lib/i386-linux-gnu/libGL.so.1 -> libGL.so.1.7.0
-rw-r--r-- 1 root root 411160 Mar 13  2019 /usr/lib/i386-linux-gnu/libGL.so.1.7.0
lrwxrwxrwx 1 root root     14 Mar 13  2019 /usr/lib/x86_64-linux-gnu/libGL.so -> libGL.so.1.7.0
lrwxrwxrwx 1 root root     14 Nov 30 19:30 /usr/lib/x86_64-linux-gnu/libGL.so.1 -> libGL.so.1.7.0
-rw-r--r-- 1 root root 596296 Mar 13  2019 /usr/lib/x86_64-linux-gnu/libGL.so.1.7.0

> LIBGL_DEBUG=verbose glxgears

> sudo apt install libglx-mesa0
Reading package lists... Done
Building dependency tree
Reading state information... Done
libglx-mesa0 is already the newest version (19.2.8-0ubuntu0~19.10.2).
libglx-mesa0 set to manually installed.
0 upgraded, 0 newly installed, 0 to remove and 0 not upgraded.

same for
libgl1-mesa-dri
```

one more try https://github.com/bincrafters/community/issues/1119

```
LIBGL_DRIVERS_PATH=/home/dpc/.conan/data/mesa/19.3.1/bincrafters/stable/package/a56cf85a12b68f87c51b8bc2331fe996caedb686/lib/dri build/dear-imgui-conan
```

#### earlies debugging of trying to remove problematic libGL.so

remove problematic `libGL.so`

```
❯ ./optixGui
MESA-LOADER: failed to open swrast (search paths /home/conan/.conan/data/mesa/19.3.1/bincrafters/stable/package/a56cf85a12b68f87c51b8bc2331fe996caedb686/lib/dri)
libGL error: failed to load driver: swrast
Glfw Error 65543: GLX: Failed to create context: BadMatch
❯ LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./optixGui


❯ strace ./optixGui >& out.log || grep GL out.log

rm /home/dpc/.conan/data/mesa/19.3.1/bincrafters/stable/package/a56cf85a12b68f87c51b8bc2331fe996caedb686/lib/libGL.so*
rm: remove symbolic link '/home/dpc/.conan/data/mesa/19.3.1/bincrafters/stable/package/a56cf85a12b68f87c51b8bc2331fe996caedb686/lib/libGL.so'? y
rm: remove symbolic link '/home/dpc/.conan/data/mesa/19.3.1/bincrafters/stable/package/a56cf85a12b68f87c51b8bc2331fe996caedb686/lib/libGL.so.1'? y
rm: remove regular file '/home/dpc/.conan/data/mesa/19.3.1/bincrafters/stable/package/a56cf85a12b68f87c51b8bc2331fe996caedb686/lib/libGL.so.1.2.0'? y

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_CUDA_FLAGS="--use_fast_math --generate-line-info" \
     -B . ..

...
-- Library GL not found in package, might be system one
...

(now will build and link)

grep -R /lib/libGL.so .

sed -i.bak '/\/lib\/libGL.so/d' ./CMakeFiles/optixGui.dir/build.make
#sed -i.bak 's/[^ ]\+\/lib\/libGL.so//g' ./CMakeFiles/optixGui.dir/link.txt
```

now should run

```
LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./optixGui
```

exporting and importing targets
-------------------------------

https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/Exporting-and-Importing-Targets#importing-targets

In this case CMake does not know the library type, so it just puts the library on the link line as-is. Therefore on Windows there is no special treatment for a shared library. The runtime library (foo.dll) need not be found. The import library (foo.lib) is specified by the IMPORTED_LOCATION property, not the IMPORTED_IMPLIB property.

sudo apt-get remove --auto-remove libgl1-mesa-dev

The following packages will be REMOVED: freeglut3-dev libdrm-dev libegl1-mesa-dev libgl1-mesa-dev libgles1 libglfw3 libglfw3-dev libglu1-mesa-dev libglvnd-core-dev libglvnd-dev libice-dev libsm-dev libvulkan-dev libwayland-bin libwayland-dev libx11-xcb-dev libxcb-dri2-0-dev libxcb-dri3-dev libxcb-glx0-dev libxcb-present-dev libxcb-randr0-dev libxcb-render0-dev libxcb-shape0-dev libxcb-sync-dev libxcb-xfixes0-dev libxdamage-dev libxrandr-dev libxshmfence-dev libxt-dev libxxf86vm-dev mesa-common-dev x11proto-damage-dev x11proto-randr-dev x11proto-xf86vidmode-dev

sudo apt install libglvnd-dev libglfw3-dev

Suggested packages: libosmesa6 libglfw3-doc libwayland-doc
