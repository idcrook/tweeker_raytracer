# Copyright (c) 2013-2020, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


cmake_minimum_required(VERSION 3.8)

project( Optix7Gui
  VERSION 0.1.0
  )
message("\nTOP PROJECT = " "${CMAKE_PROJECT_NAME}")
message("PROJECT_NAME = " "${PROJECT_NAME}")

cmake_policy(VERSION 3.8)

if( POLICY CMP0072 )
  # FindOpenGL prefers GLVND by default when available
  cmake_policy(SET CMP0072 NEW)
  # # the OLD behavior for this policy is to set OpenGL_GL_PREFERENCE to LEGACY
  # cmake_policy(SET CMP0072 OLD)
endif()


set(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/../CMake"
  ${CMAKE_MODULE_PATH}
  )

message("CMAKE_PREFIX_PATH    = " "${CMAKE_PREFIX_PATH}")
message("CMAKE_MODULE_PATH    = " "${CMAKE_MODULE_PATH}")
message("CMAKE_BUILD_TYPE     = " "${CMAKE_BUILD_TYPE}")

# Set to c++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Currently this project uses the deprecated CUDA find_package
include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
  enable_language(CUDA)
  if(NOT DEFINED CMAKE_CUDA_STANDARD)
    set(CMAKE_CUDA_STANDARD 11)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
  endif()
  # requires cmake 3.17 - FindCUDAToolkit()
  find_package(CUDA 10.0 REQUIRED)  # Still required even with CUDA language
                                    # support so we can link against CUDA from
                                    # a cpp-only target

  # If you want to use separable compilation you will need to use
  # CMAKE_CUDA_FLAGS as target_compile_options aren’t propagated when doing
  # device linking
  #set(CMAKE_CUDA_SEPARABLE_COMPILATION ON) # for multi-target pre-compilations

  INCLUDE(configure_optix7)
else()
  message(FATAL_ERROR "CMake did not find CUDA, which is required to build this program.")
endif()

# see above for GLVND note
find_package(OpenGL REQUIRED)

##################################################################
# conan config and install dependencies
#
# - requires conan to be installed before generating with CMake
#

# With multi generators, conan_cmake_run expects a build_type to be provided
if(UNIX AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
# WARNING: CMake caches most recent explicit CLI -DCMAKE_BUILD_TYPE for this option

# https://docs.conan.io/en/latest/reference/generators/cmakemulti.html?highlight=message#conan-cmake-silent-output
set(CONAN_CMAKE_SILENT_OUTPUT ON)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

# remote needed to workaround for busted main repo libx11 dependency from glfw
##ERROR: Failed requirement 'libx11/1.6.8@bincrafters/stable' from 'glfw/3.3.2@bincrafters/stable'
##ERROR: Unable to find 'libx11/1.6.8@bincrafters/stable' in remotes

# conan_add_remote(
#   NAME bincrafters
#   URL https://api.bintray.com/conan/bincrafters/public-conan
#   )

# BUILD missing needed on -DCMAKE_BUILD_TYPE=Release due to:
# mesa-glu/9.0.1@bincrafters/stable: WARN: Can't find a 'mesa-glu/9.0.1@bincrafters/stable' package for the specified settings, options and dependencies:

# https://docs.conan.io/en/latest/reference/generators/cmake_find_package_multi.html
conan_cmake_run(
  CONANFILE conanfile.txt
  BASIC_SETUP
  BUILD missing
  )

# these are "imports" in conanfile.txt
set( IMGUI_BINDINGS
  bindings/imgui_impl_glfw.h
  bindings/imgui_impl_glfw.cpp
  bindings/imgui_impl_opengl3.h
  bindings/imgui_impl_opengl3.cpp
)

# copy the conan outputs (headers and source for imgui bindings)
message(STATUS "Copying imgui bindings")
FOREACH(input ${IMGUI_BINDINGS})
  get_filename_component(DEST_PATH "${input}" ABSOLUTE)

  # the CONAN_RESULT is relative to the current binary directory
  string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_BINARY_DIR}/" PATH_CONAN_RESULT "${DEST_PATH}")
  # message("\nPATH_CONAN_RESULT    = " " ${PATH_CONAN_RESULT}")
  # message("DEST_PATH            = " " ${DEST_PATH}")

  # configure_file respects Makefile timestamp dynamics
  configure_file(${PATH_CONAN_RESULT} ${DEST_PATH} COPYONLY)

ENDFOREACH( )

# needed for local conan link_directories  imgui::imgui, etc.
set(CMAKE_PREFIX_PATH
  # "${CMAKE_CURRENT_SOURCE_DIR}/build"
  "${CMAKE_CURRENT_BINARY_DIR}"  # works when called locally or from top-level
  ${CMAKE_PREFIX_PATH}
  )

# related to conan: CONFIG option is important so CMake does not search for
# modules into the default modules directory
find_package(imgui  CONFIG REQUIRED)
#find_package(glfw   CONFIG REQUIRED) # switch to using system package instead
#find_package(glew   CONFIG REQUIRED) # switch to using system package instead

#find_package(DevIL_1_8_0 REQUIRED)
#find_package(DevIL_1_7_8 REQUIRED)
find_package(DevIL REQUIRED)  # 1.7.8 is the system package version on ubuntu

# TBD Usage report verbosity - Currently unimplemented
set(OPTIX7GUI_USAGE_REPORT_VERBOSITY "0" CACHE STRING "Verbosity of OptiX usage report (0 disables reporting).")
set_property(CACHE OPTIX7GUI_USAGE_REPORT_VERBOSITY PROPERTY STRINGS 0 1 2 3)

option(OPTIX7GUI_USE_DEBUG_EXCEPTIONS "Enables advanced exception handling and error checking for debugging purposes." OFF)

# NOTE below, without "--relocatable-device-code=true" flag, will receive warning:
# /home/dpc/projects/learning/rt/tweeker_raytracer/apps/Optix7Gui/shaders/miss.cu(38): warning: extern declaration of the entity sysParameter is treated as a static definition

# Flags to compile kernels to PTX
if (OPTIX7GUI_USE_DEBUG_EXCEPTIONS)
  set(local_cuda_flags
    #"--machine=64"
    "--gpu-architecture=compute_30"
    "--relocatable-device-code=true"
    "--use_fast_math"
    "--keep-device-functions"
    "--generate-line-info"
    "--verbose"
    )
else()
  set(local_cuda_flags
    #"--machine=64"
    #"-arch=sm_30"
    "--gpu-architecture=compute_30"
    "--relocatable-device-code=true"
    "--use_fast_math"
    "--keep-device-functions"
    "--generate-line-info"
    )
endif()

string(REPLACE ";" " " CMAKE_CUDA_FLAGS "${local_cuda_flags}" )
message("CMAKE_CUDA_FLAGS    = " "${CMAKE_CUDA_FLAGS}")


set( HEADERS
  inc/Application.h
  inc/CheckMacros.h
  inc/Logger.h
  inc/MyAssert.h
  inc/Options.h
  inc/Picture.h
  inc/PinholeCamera.h
  inc/Texture.h
  inc/Timer.h
)

set( SOURCES
  src/main.cpp
  src/Application.cpp
  src/Box.cpp
  src/Options.cpp
  src/Parallelogram.cpp
  src/Picture.cpp
  src/PinholeCamera.cpp
  src/Plane.cpp
  src/Torus.cpp
  src/Sphere.cpp
  src/Texture.cpp
  src/Timer.cpp
)

# Prefix the shaders with the full path name to allow stepping through errors with F8.
set( CUDA_SHADERS
  # Core shaders.
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/anyhit.cu
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/closesthit.cu
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/raygeneration.cu
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/exception.cu
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/miss.cu

  # Direct callables
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/lens_shader.cu
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/light_sample.cu
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/bsdf_diffuse_reflection.cu
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/bsdf_specular_reflection.cu
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/bsdf_specular_reflection_transmission.cu
)

set( CUDA_SHADERS_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/app_config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/function_indices.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/half_common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/light_definition.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/material_parameter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/per_ray_data.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/random_number_generators.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/system_parameter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader_common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vector_math.h
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex_attributes.h
)


# CUDA_SHADERS_HEADERS are colocated in same directory
add_library(shadersPTX OBJECT ${CUDA_SHADERS})
set_property(TARGET shadersPTX PROPERTY CUDA_PTX_COMPILATION ON)
target_include_directories(shadersPTX PUBLIC   ${OPTIX7_INCLUDE_DIR})
# where PTX.h will get placed
target_include_directories(shadersPTX PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)

# Retrieve bin2c util
get_filename_component(cuda_compiler_bin "${CMAKE_CUDA_COMPILER}" DIRECTORY)
find_program(bin_to_c
    NAMES bin2c
    PATHS ${cuda_compiler_bin}
    )
if(NOT bin_to_c)
    message(FATAL_ERROR
        "bin2c not found:\n"
        "  CMAKE_CUDA_COMPILER='${CMAKE_CUDA_COMPILER}'\n"
        "  cuda_compiler_bin='${cuda_compiler_bin}'\n"
    )
endif()

# Embed PTX in header file that will be built into program
set(headerPathPTX ${CMAKE_CURRENT_BINARY_DIR}/include/PTX.h)
add_custom_command(
    OUTPUT "${headerPathPTX}"
    COMMAND ${CMAKE_COMMAND}
    "-DBIN_TO_C_COMMAND=${bin_to_c}"
    "-DOBJECTS=$<TARGET_OBJECTS:shadersPTX>"
    "-DOUTPUT=${headerPathPTX}"
    -P ${PROJECT_SOURCE_DIR}/../CMake/bin2c_wrapper.cmake
    VERBATIM
    DEPENDS $<TARGET_OBJECTS:shadersPTX>
    COMMENT "Converting PTX files to a C header" "${headerPathPTX}"
)


add_executable( optix7Gui
  ${IMGUI_BINDINGS}
  ${HEADERS}
  ${SOURCES}
  ${headerPathPTX}
  )

add_dependencies(optix7Gui shadersPTX)

message("CUDA_TOOLKIT_INCLUDE  = " "${CUDA_TOOLKIT_INCLUDE}")
#message("CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES  = " "${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")

# add_definitions(
#   # Disable warnings for file operations fopen etc.
#   "-D_CRT_SECURE_NO_WARNINGS"
# )
target_compile_definitions(optix7Gui PRIVATE "_CRT_SECURE_NO_WARNINGS")

# are using GLEW opengl loader
target_compile_definitions(optix7Gui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)


target_include_directories(optix7Gui PUBLIC
  "."
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  # $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)
target_include_directories(optix7Gui PRIVATE
  ${CUDA_TOOLKIT_INCLUDE}
  ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
  ${OPTIX7_INCLUDE_DIR}
  ${IL_INCLUDE_DIR}
)


target_link_libraries( optix7Gui PUBLIC
  OpenGL::OpenGL # Defined to libOpenGL if the system is GLVND-based. OpenGL::GL
  imgui::imgui
  # glew::glew # switch to using system package instead
  # glfw::glfw # switch to using system package instead
  GLEW  # v2.1
  glfw  # v3.3
  #${CUDA_LIBRARIES} # runtime libraries
  ${CUDA_CUDA_LIBRARY} # driver library (for OptiX7)
  ${IL_LIBRARIES}
  ${ILU_LIBRARIES}
  ${ILUT_LIBRARIES}
)

if (UNIX)
  target_link_libraries( optix7Gui PRIVATE dl ${CMAKE_DL_LIBS})
endif()

#
message("OPENGL_gl_LIBRARY     = " " ${OPENGL_gl_LIBRARY}")
message("OPENGL_opengl_LIBRARY = " " ${OPENGL_opengl_LIBRARY}")
message("OPENGL_glx_LIBRARY    = " " ${OPENGL_glx_LIBRARY}")
message("OPENGL_LIBRARIES      = " " ${OPENGL_LIBRARIES}")
#
# message("CUDA_CUDA_LIBRARY     = " " ${CUDA_CUDA_LIBRARY}")
message("CUDA_LIBRARIES        = " " ${CUDA_LIBRARIES}")

if ("${CMAKE_BINARY_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
  set(relative "./")
else()
  set(relative "build/")
endif()

# print out
string(REPLACE "${CMAKE_SOURCE_DIR}/build/" ${relative} target_bin "${CMAKE_CURRENT_BINARY_DIR}/bin/optix7Gui")

message(STATUS "Will build ${target_bin}")

add_custom_command(TARGET optix7Gui POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan " ${target_bin}")
