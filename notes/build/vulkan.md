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
