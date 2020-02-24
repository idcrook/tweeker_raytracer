cmake_minimum_required ( VERSION 3.8 )

project ( OptixGui VERSION 0.1.0  )
message("\nPROJECT_NAME = " "${PROJECT_NAME}")
message("CMAKE_CURRENT_BINARY_DIR = " "${CMAKE_CURRENT_BINARY_DIR}")

# set to match your path
set(OptiX_INSTALL_DIR "/usr/local/nvidia/NVIDIA-OptiX-SDK-6.5.0-linux64/"
  CACHE PATH "Path to OptiX installed location.")
# can override on cmake command line with: -DOptiX_INSTALL_DIR="/path/to/NVIDIA-OptiX-SDK-6.5.0-linux64/" \


# As of CMake 2.6 policies were introduced in order to provide a mechanism for
cmake_policy(VERSION 3.8)

if( POLICY CMP0072 )
  # FindOpenGL prefers GLVND by default when available
  cmake_policy(SET CMP0072 NEW)
endif()

set(CMAKE_MODULE_PATH
  #    "${PROJECT_SOURCE_DIR}/CMake"
  "${CMAKE_CURRENT_SOURCE_DIR}/../CMake"
  ${CMAKE_MODULE_PATH}
  )

# Set to c++11
set ( CMAKE_CXX_STANDARD 11 )

include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
  enable_language(CUDA)
  if(NOT DEFINED CMAKE_CUDA_STANDARD)
    set(CMAKE_CUDA_STANDARD 11)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
  endif()
  #set(CMAKE_CUDA_SEPARABLE_COMPILATION ON) # for multi-target pre-compilations
  find_package(CUDA 7.0 REQUIRED)  # added when trying to debug RT_CALLABLE_PROGRAM missing

  # Defines OptiX utilities and variables
  INCLUDE(configure_optix)
else()
  message(STATUS "No CUDA support")
endif()


##################################################################
# conan config and install dependencies
#

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

conan_add_remote(
  NAME bincrafters
  URL https://api.bintray.com/conan/bincrafters/public-conan
  )

# BUILD missing needed on -DCMAKE_BUILD_TYPE=Release due to:
# mesa-glu/9.0.1@bincrafters/stable: WARN: Can't find a 'mesa-glu/9.0.1@bincrafters/stable' package for the specified settings, options and dependencies:

conan_cmake_run(
  CONANFILE conanfile.txt
  BASIC_SETUP
  BUILD missing
  )


# needed for local conan link_directories  imgui::imgui, etc.
set(CMAKE_PREFIX_PATH
  # "${CMAKE_CURRENT_SOURCE_DIR}/build"
  "${CMAKE_CURRENT_BINARY_DIR}"  # works when called locally or from top-level
  ${CMAKE_PREFIX_PATH}
  )

#message("CMAKE_PREFIX_PATH    = " "${CMAKE_PREFIX_PATH}")
#message("CMAKE_CURRENT_BINARY_DIR = " "${CMAKE_CURRENT_BINARY_DIR}")
#message("CMAKE_MODULE_PATH    = " "${CMAKE_MODULE_PATH}")


# find_package will search CONFIG before MODULE with this turned ON
#set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# debug include_directories

# get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
# foreach(dir ${dirs})
#   message(STATUS "dir='${dir}'")
# endforeach()


# These dependencies are installed using conan, CONFIG is implied by FIND_PACKAGE_PREFER_CONFIG ON
find_package(imgui CONFIG REQUIRED)
find_package(glfw  CONFIG REQUIRED)
find_package(glew  CONFIG REQUIRED)

# see above for GLVND note
find_package(OpenGL REQUIRED)

# image loading - expect system developer package to be installed
find_package(DevIL REQUIRED)


if ( OPENGL_FOUND AND OPENGL_INCLUDE_DIR )
  include_directories( ${OPENGL_INCLUDE_DIR} )
endif()

# message("OPENGL_FOUND          = " " ${OPENGL_FOUND}")
# message("OPENGL_INCLUDE_DIR    = " " ${OPENGL_INCLUDE_DIR}")
# message("OPENGL_glx_LIBRARY    = " " ${OPENGL_glx_LIBRARY}")
# message("OPENGL_opengl_LIBRARY = " " ${OPENGL_opengl_LIBRARY}")
# message("OPENGL_gl_LIBRARY     = " " ${OPENGL_gl_LIBRARY}")

# for local source includes and devil
INCLUDE_DIRECTORIES(
  "."
  ${IL_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/build
  )

# # Tells CMake to build all the libraries as shared libraries by default.  This can be
# # overrided by individual libraries later.
# set(BUILD_SHARED_LIBS ON)

##################################################################
# CUDA config

# Present the CUDA_64_BIT_DEVICE_CODE on the default set of options.
mark_as_advanced(CLEAR CUDA_64_BIT_DEVICE_CODE)

# Add some useful default arguments to the nvcc flags.  This is an example of how we use
# PASSED_FIRST_CONFIGURE.  Once you have configured, this variable is TRUE and following
# block of code will not be executed leaving you free to edit the values as much as you
# wish from the GUI or from ccmake.
if(NOT PASSED_FIRST_CONFIGURE)

  list(APPEND default_nvcc_flags "--use_fast_math" "-arch sm_30" "-lineinfo")

  if (CUDA_VERSION VERSION_LESS "3.0")
    list(APPEND default_nvcc_flags "--keep")
  endif()

  if( APPLE )
    # Undef'ing __BLOCKS__ for OSX builds.  This is due to a name clash between OSX 10.6
    # C headers and CUDA headers
    list(APPEND default_nvcc_flags "-U__BLOCKS__")
  endif()

  # Append to nvcc flags
  foreach(flag IN LISTS default_nvcc_flags)
    list(FIND CUDA_NVCC_FLAGS ${flag} index)
    if(index EQUAL -1)
      list(APPEND CUDA_NVCC_FLAGS ${flag})
      set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} CACHE LIST "Semi-colon delimit multiple arguments." FORCE)
    endif()
  endforeach(flag)

endif(NOT PASSED_FIRST_CONFIGURE)

# Add required nvcc flag for callable programs under CUDA 8
if (CUDA_VERSION VERSION_GREATER "7.5")
  set(flag "--keep-device-functions")
  list(FIND CUDA_NVCC_FLAGS ${flag} index)
  if(index EQUAL -1)
    list(APPEND CUDA_NVCC_FLAGS ${flag})
    set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} CACHE LIST "Semi-colon delimit multiple arguments." FORCE)
  endif()
endif()

##################################################################
# SUtil compilation
#
# - needs OptiX to be found before here
#
# - removed glew.h symbols deleted on transition from GLEW 2.0 -> 2.1
#
# - move after conan, so uses GLEW headers from 2.1 dependency
#

message("OptiX_INCLUDE    = " "${OptiX_INCLUDE}")

set(SAMPLES_PTX_DIR "${CMAKE_BINARY_DIR}/lib/ptx")
set(SAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

message("SAMPLES_PTX_DIR    = " "${SAMPLES_PTX_DIR}")
message("SAMPLES_DIR        = " "${SAMPLES_DIR}")

set(CUDA_GENERATED_OUTPUT_DIR ${SAMPLES_PTX_DIR})

if (WIN32)
  string(REPLACE "/" "\\\\" SAMPLES_PTX_DIR ${SAMPLES_PTX_DIR})
else (WIN32)
  if ( USING_GNU_C AND NOT APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DM_PI=3.14159265358979323846" )
  endif()
endif (WIN32)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sampleConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/sampleConfig.h @ONLY)

# Path to sutil.h that all the samples need
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/sutil
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OptiX_INCLUDE}/optixu
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CUDA_INCLUDE_DIRS} )


set(SAMPLES_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/device_include)


#########################################################
# OPTIX_add_sample_executable
#
# Convience function for adding samples to the code.  You can copy the contents of this
# funtion into your individual project if you wish to customize the behavior.  Note that
# in CMake, functions have their own scope, whereas macros use the scope of the caller.
function(OPTIX_add_sample_executable target_name)

  # These calls will group PTX and CUDA files into their own directories in the Visual
  # Studio projects.
  source_group("PTX Files"  REGULAR_EXPRESSION ".+\\.ptx$")
  source_group("CUDA Files" REGULAR_EXPRESSION ".+\\.cu$")

  # Separate the sources from the CMake and CUDA options fed to the macro.  This code
  # comes from the CUDA_COMPILE_PTX macro found in FindCUDA.cmake.  We are copying the
  # code here, so that we can use our own name for the target.  target_name is used in the
  # creation of the output file names, and we want this to be unique for each target in
  # the SDK.
  CUDA_GET_SOURCES_AND_OPTIONS(source_files cmake_options options ${ARGN})

  # Create the rules to build the PTX from the CUDA files.
  CUDA_WRAP_SRCS( ${target_name} PTX generated_files ${source_files} ${cmake_options}
    OPTIONS ${options} )

  # Here is where we create the rule to make the executable.  We define a target name and
  # list all the source files used to create the target.  In addition we also pass along
  # the cmake_options parsed out of the arguments.
  add_executable(${target_name}
    ${source_files}
    ${generated_files}
    ${cmake_options}
    )

  # Most of the samples link against sutil, optix, and glfw.  Here is the
  # rule that specifies this linkage.
  target_link_libraries(  ${target_name}
    sutil_sdk
    # OpenGL::GL
    OpenGL::OpenGL # Defined to libOpenGL if the system is GLVND-based. OpenGL::GL
    imgui::imgui
    glew::glew
    glfw::glfw
    ${optix_LIBRARY}
    ${IL_LIBRARIES}
    ${ILU_LIBRARIES}
    ${ILUT_LIBRARIES}
    )

  message("optix_LIBRARY =      " "${optix_LIBRARY}")
  message("OPENGL_opengl_LIBRARY = " " ${OPENGL_opengl_LIBRARY}")

  # are using GLEW opengl loader
  target_compile_definitions( ${target_name} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)

  if(USING_GNU_CXX)
    target_link_libraries( ${target_name} m ) # Explicitly link against math library (C samples don't do that by default)
  endif()
endfunction()



# Our sutil library.  The rules to build it are found in the subdirectory.
add_subdirectory(sutil)

OPTIX_add_sample_executable( optixGui
  main.cpp

  include/Application.h
  Application.cpp

  include/Options.h
  Options.cpp

  Box.cpp
  Parallelogram.cpp
  Plane.cpp
  Sphere.cpp
  Torus.cpp

  include/LensShader.h

  include/Picture.h
  Picture.cpp

  include/PinholeCamera.h
  PinholeCamera.cpp

  include/Texture.h
  Texture.cpp

  include/Timer.h
  Timer.cpp

  include/MyAssert.h

  shaders/app_config.cuh

  shaders/function_indices.cuh
  shaders/light_definition.cuh
  shaders/material_parameter.cuh
  shaders/per_ray_data.cuh
  shaders/random_number_generators.cuh
  shaders/rt_assert.cuh
  shaders/rt_function.cuh
  shaders/shader_common.cuh
  shaders/vertex_attributes.cuh

  shaders/boundingbox_triangle_indexed.cu
  shaders/intersection_triangle_indexed.cu

  shaders/raygeneration.cu
  shaders/miss.cu
  shaders/exception.cu

  shaders/closesthit.cu
  shaders/closesthit_light.cu

  shaders/anyhit.cu

  shaders/lens_shader.cu
  shaders/bsdf_diffuse_reflection.cu
  shaders/bsdf_specular_reflection.cu
  shaders/bsdf_specular_reflection_transmission.cu
  shaders/light_sample.cu

  bindings/imgui_impl_glfw.h
  bindings/imgui_impl_glfw.cpp
  bindings/imgui_impl_opengl3.h
  bindings/imgui_impl_opengl3.cpp
)
