cmake_minimum_required ( VERSION 3.11 )

project ( OptixGui VERSION 0.1.0  )

# # set to match your path
# set(OptiX_INSTALL_DIR "/usr/local/nvidia/NVIDIA-OptiX-SDK-6.5.0-linux64/"
#   CACHE PATH "Path to OptiX installed location.")
# can set on cmake command line with: -DOptiX_INSTALL_DIR="/path/to/NVIDIA-OptiX-SDK-6.5.0-linux64/" \


# needed for conan link_directories  imgui::imgui, etc.
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build")

# As of CMake 2.6 policies were introduced in order to provide a mechanism for
cmake_policy(VERSION 3.11)

if( POLICY CMP0072 )
  # FindOpenGL prefers GLVND by default when available
  cmake_policy(SET CMP0072 NEW)
endif()

if( POLICY CMP0074 )
  # find_package uses <PackageName>_ROOT variables.
  cmake_policy(SET CMP0074 NEW)
endif()

set(CMAKE_MODULE_PATH
  #    "${PROJECT_SOURCE_DIR}/CMake"
  "${CMAKE_CURRENT_SOURCE_DIR}/../CMake"
  ${CMAKE_MODULE_PATH}
  )

#message(STATUS "CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}")

# for local source includes and devil
INCLUDE_DIRECTORIES(
  "."
  ${IL_INCLUDE_DIR}
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

  # Defines OptiX utilities and variables
  INCLUDE(configure_optix)
else()
  message(STATUS "No CUDA support")
endif()

# related to conan
# CONFIG option is important so CMake does not search for modules into the default modules directory
find_package(imgui CONFIG)
find_package(glfw CONFIG)
find_package(glad CONFIG)

# see above for GLVND note
find_package(OpenGL REQUIRED)

#
find_package(DevIL REQUIRED)


cuda_compile_and_embed( raygen_ptx_c   raygen/raygen.cu )
cuda_compile_and_embed(   miss_ptx_c     raygen/miss.cu )

cuda_compile_and_embed(  sphere_ptx_c          geometry/sphere.cu )
cuda_compile_and_embed( moving_sphere_ptx_c   geometry/movingSphere.cu )
cuda_compile_and_embed( volume_sphere_ptx_c   geometry/volumeSphere.cu )
cuda_compile_and_embed( aarect_ptx_c          geometry/aarect.cu )
cuda_compile_and_embed( volume_box_ptx_c      geometry/volumeBox.cu )

cuda_compile_and_embed( normal_material_ptx_c     material/normalMaterial.cu  )
cuda_compile_and_embed( lambertian_material_ptx_c material/lambertianMaterial.cu  )
cuda_compile_and_embed( metal_material_ptx_c      material/metalMaterial.cu  )
cuda_compile_and_embed( dielectric_material_ptx_c material/dielectricMaterial.cu  )
cuda_compile_and_embed( diffuse_light_material_ptx_c material/diffuseLight.cu  )
cuda_compile_and_embed( isotropic_material_ptx_c  material/isotropicMaterial.cu  )

cuda_compile_and_embed( null_texture_ptx_c        texture/nullTexture.cu  )
cuda_compile_and_embed( constant_texture_ptx_c    texture/constantTexture.cu  )
cuda_compile_and_embed( checkered_texture_ptx_c   texture/checkeredTexture.cu  )
cuda_compile_and_embed( noise_texture_ptx_c       texture/noiseTexture.cu  )
cuda_compile_and_embed( image_texture_ptx_c       texture/imageTexture.cu  )

cuda_compile_and_embed( cosine_pdf_ptx_c  pdf/cosinePdf.cu )
cuda_compile_and_embed( rect_pdf_ptx_c    pdf/rectPdf.cu )
cuda_compile_and_embed( mixture_pdf_ptx_c      pdf/mixturePdf.cu )
cuda_compile_and_embed( mixture_bias_pdf_ptx_c pdf/mixtureBiasPdf.cu )


##################################################################
# SUtil compilation

set(SAMPLES_PTX_DIR "${CMAKE_BINARY_DIR}/lib/ptx")
set(SAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

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

# Our sutil library.  The rules to build it are found in the subdirectory.
add_subdirectory(sutil)


add_executable(optixGui
  main.cpp

  Director.h
  Director.cpp

  Application.h
  Application.cpp

  Box.cpp
  Parallelogram.cpp
  Plane.cpp
  Sphere.cpp
  Torus.cpp

  Texture.h
  Texture.cpp

  Timer.h
  Timer.cpp

  PinholeCamera.h
  PinholeCamera.cpp

  Picture.h
  Picture.cpp

  bindings/imgui_impl_glfw.cpp
  bindings/imgui_impl_opengl3.cpp
  bindings/imgui_impl_glfw.h
  bindings/imgui_impl_opengl3.h

  lib/app_config.cuh

  # Compiled cuda ptx programs
  ${raygen_ptx_c}
  ${miss_ptx_c}

  ${sphere_ptx_c}
  ${moving_sphere_ptx_c}
  ${aarect_ptx_c}
  ${volume_box_ptx_c}
  ${volume_sphere_ptx_c}

  ${cosine_pdf_ptx_c}
  ${rect_pdf_ptx_c}
  ${mixture_pdf_ptx_c}
  ${mixture_bias_pdf_ptx_c}

  ${normal_material_ptx_c}
  ${lambertian_material_ptx_c}
  ${metal_material_ptx_c}
  ${dielectric_material_ptx_c}
  ${diffuse_light_material_ptx_c}
  ${isotropic_material_ptx_c}

  ${null_texture_ptx_c}
  ${constant_texture_ptx_c}
  ${checkered_texture_ptx_c}
  ${noise_texture_ptx_c}
  ${image_texture_ptx_c}
  )


target_compile_definitions(optixGui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

target_link_libraries(optixGui
  sutil_sdk
  OpenGL::GL
  imgui::imgui
  glad::glad
  glfw::glfw
  ${optix_LIBRARY}
  ${IL_LIBRARIES}
  ${ILU_LIBRARIES}
  ${ILUT_LIBRARIES}
  )
