install vulkan SDK
==================

on Ubuntu 19.10

fropm https://vulkan.lunarg.com/sdk/home

Linux -> Ubuntu Packages

-	Ubuntu 18.04 (Bionic Beaver)

copied from the isntructions and ran

```
wget -qO - http://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.2.131-bionic.list http://packages.lunarg.com/vulkan/1.2.131/lunarg-vulkan-1.2.131-bionic.list
sudo apt update
sudo apt install vulkan-sdk
```

from the output

```
The following additional packages will be installed:
  glslang-dev glslang-tools libqt5quickwidgets5 libqt5webengine-data libqt5webenginecore5 libqt5webenginewidgets5 libre2-5
  libvulkan-dev libvulkan1 libxcb-ewmh2 lunarg-via lunarg-vkconfig lunarg-vktrace lunarg-vulkan-layers shaderc spirv-cross
  spirv-headers spirv-tools vulkan-headers vulkan-tools vulkan-validationlayers vulkan-validationlayers-dev
The following NEW packages will be installed:
  glslang-dev glslang-tools libqt5quickwidgets5 libqt5webengine-data libqt5webenginecore5 libqt5webenginewidgets5 libre2-5
  libxcb-ewmh2 lunarg-via lunarg-vkconfig lunarg-vktrace lunarg-vulkan-layers shaderc spirv-cross spirv-headers spirv-tools
  vulkan-headers vulkan-sdk vulkan-tools vulkan-validationlayers vulkan-validationlayers-dev
The following packages will be upgraded:
  libvulkan-dev libvulkan1
2 upgraded, 21 newly installed, 0 to remove and 0 not upgraded.
```

Uninstalling the SDK
--------------------

```shell
sudo apt purge vulkan-sdk
sudo apt autoremove
```

install beta vulkan Nvidia driver (vulkan 1.2)
----------------------------------------------

https://www.khronos.org/vulkan/

download from https://developer.nvidia.com/vulkan-driver

```
sh ./NVIDIA-Linux-x86_64-440.58.02.run --info
sudo sh  ./NVIDIA-Linux-x86_64-440.58.02.run  --driver-info
```

will need to install from single-user mode

-	logout of desktop
-	switch to virtual console: <Ctrl>\-<Alt>\-<kbd>F2</kbd>
-	uninstall ubuntu ppa driver:

`sudo apt remove nvidia-driver-440`

```
sudo service gdm3 stop
sudo su -
# now running as root
cd ~dpc/Downloads/nvidia/vk-beta
CC=gcc-9 sh ./NVIDIA*
# 440.58.02
# no DKMS
```

install ubuntu distributed driver

```
sudo ubuntu-drivers devices
sudo apt install nvidia-driver-440
```

Download from nvidia: https://www.nvidia.com/en-us/drivers/unix/

Version: 440.64 Release Date: 2020.2.28

PPA: https://launchpad.net/~graphics-drivers/+archive/ubuntu/ppa

version at writing:

https://launchpad.net/ubuntu/+source/nvidia-graphics-drivers-440/440.59-0ubuntu0.19.10.1

Run the examples Applications
-----------------------------

Verify the executables named `vkcube`, `vkcubepp`, and `vulkaninfo` are present. e.g.

```
which vkcube
```

Run the example applications. e.g.

```shell
vkcube
vkcubepp
vulkaninfo
```

Vulkan configuration tool
-------------------------

```shell
vkconfig &
```
