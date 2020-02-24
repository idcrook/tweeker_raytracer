# ======================================================================== #
# Copyright 2018 Ingo Wald                                                 #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ======================================================================== #

# message(STATUS "OptiX_INSTALL_DIR: ${OptiX_INSTALL_DIR}")
# message(STATUS "PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")
# message(STATUS "CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
# message(STATUS "CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}")
# message(STATUS "CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}")

if (NOT DEFINED CUDA_FOUND)
  find_package(CUDA 10.0 REQUIRED)
endif()

find_package(OptiX7 REQUIRED)

# #include_directories(${CUDA_TOOLKIT_INCLUDE})
# if (CUDA_TOOLKIT_ROOT_DIR)
# 	include_directories(${CUDA_TOOLKIT_ROOT_DIR}/include)
# endif()
# include_directories(${OptiX_INCLUDE})

message(STATUS "Found CUDA ${CUDA_VERSION_STRING} at ${CUDA_TOOLKIT_ROOT_DIR}")
#message("CUDA_NVCC_FLAGS          = " " ${CUDA_NVCC_FLAGS}")
message("CMAKE_CUDA_EXECUTABLE                  = " " ${CMAKE_CUDA_EXECUTABLE}")
message("CMAKE_CUDA_COMPILER                    = " " ${CMAKE_CUDA_COMPILER}")
message("CMAKE_CUDA_FLAGS                       = " " ${CMAKE_CUDA_FLAGS}")
message("CMAKE_CUDA_HOST_COMPILER               = " " ${CMAKE_CUDA_HOST_COMPILER}")
message("CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES = " " ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")

# message("CUDA_TOOLKIT_INCLUDE_DIRECTORIES = " " ${CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")
# message("CUDA_HOST_COMPILER       = " " ${CUDA_HOST_COMPILER}")
# message("CUDA_TOOLKIT_INCLUDE_DIRECTORIES = " " ${CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")
message("CUDA_VERSION                    = " " ${CUDA_VERSION}")

# https://developer.download.nvidia.com/video/gputechconf/gtc/2019/presentation/s9444-build-systems-exploring-modern-cmake-cuda-v2.pdf
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
#set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler=-Wall")
#set(cuda_flags "-Xcudafe=--display_error_number")

# if (WIN32)
#   add_definitions(-DNOMINMAX)
# endif()

# Use the NVCUDA_COMPILE_PTX function to produce the desired custom rule and output filenames when compiling OptiX programs from *.cu to *.ptx.
#include("nvcuda_compile_ptx")

# if(UNIX)
#   set(OS "linux")
#   add_definitions("-DLINUX")
#   add_definitions("-Wno-unused-local-typedefs -Wno-delete-non-virtual-dtor")
#   SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
# else(UNIX)
#   if(APPLE)
#   else(APPLE)
#     if(WIN32)
#       set(OS "win")
#       add_definitions("-DNOMINMAX")
#     endif(WIN32)
#   endif(APPLE)
# endif(UNIX)



# find_program(BIN2C bin2c
#   DOC "Path to the cuda-sdk bin2c executable.")

# # this macro defines cmake rules that execute the following four steps:
# # 1) compile the given cuda file ${cuda_file} to an intermediary PTX file
# # 2) use the 'bin2c' tool (that comes with CUDA) to
# #    create a second intermediary (.c-)file which defines a const string variable
# #    (named '${c_var_name}') whose (constant) value is the PTX output
# #    from the previous step.
# # 3) compile the given .c file to an intermediary object file (why thus has
# #    that PTX string 'embedded' as a global constant.
# # 4) assign the name of the intermediary .o file to the cmake variable
# #    'output_var', which can then be added to cmake targets.
# macro(cuda_compile_and_embed output_var cuda_file)
#   set(c_var_name ${output_var})
#   #cuda_compile_ptx(ptx_files ${cuda_file})
#   #cuda_compile_ptx(ptx_files ${cuda_file} OPTIONS --relocatable-device-code=true )
#   cuda_compile_ptx(ptx_files ${cuda_file} OPTIONS --relocatable-device-code=true;--use_fast_math; )
#   #cuda_compile_ptx(ptx_files ${cuda_file} OPTIONS --gpu-architecture=compute_75;--use_fast_math;--relocatable-device-code=true;--verbose )
#   #cuda_compile_ptx(ptx_files ${cuda_file} OPTIONS --gpu-architecture=compute_75 )
#   list(GET ptx_files 0 ptx_file)
#   set(embedded_file ${ptx_file}_embedded.c)
#   #  message("adding rule to compile and embed ${cuda_file} to \"const char ${var_name}[];\"")
#   add_custom_command(
#     OUTPUT ${embedded_file}
#     COMMAND ${BIN2C} -c --padd 0 --type char --name ${c_var_name} ${ptx_file} > ${embedded_file}
#     DEPENDS ${ptx_file}
#     COMMENT "compiling (and embedding ptx from) ${cuda_file}"
#     )
#   set(${output_var} ${embedded_file})
# endmacro()

# include_directories(${OptiX_INCLUDE})
# #message(STATUS "OptiX includes: ${OptiX_INCLUDE}")

# add_definitions(-D__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__=1)
