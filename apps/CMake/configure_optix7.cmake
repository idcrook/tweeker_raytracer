
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

# message(STATUS "Found CUDA ${CUDA_VERSION_STRING} at ${CUDA_TOOLKIT_ROOT_DIR}")
message("CMAKE_CUDA_COMPILER   = " " ${CMAKE_CUDA_COMPILER}")
message("CUDA_VERSION          = " " ${CUDA_VERSION}")
message("CUDA_TOOLKIT_INCLUDE  = " " ${CUDA_TOOLKIT_INCLUDE}")


# #message("CUDA_NVCC_FLAGS          = " " ${CUDA_NVCC_FLAGS}")
# message("CMAKE_CUDA_EXECUTABLE                  = " " ${CMAKE_CUDA_EXECUTABLE}")
# message("CMAKE_CUDA_FLAGS                       = " " ${CMAKE_CUDA_FLAGS}")
# message("CMAKE_CUDA_HOST_COMPILER               = " " ${CMAKE_CUDA_HOST_COMPILER}")
# message("CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES = " " ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")
# message("CUDA_TOOLKIT_INCLUDE_DIRECTORIES = " " ${CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")
# message("CUDA_HOST_COMPILER       = " " ${CUDA_HOST_COMPILER}")
# message("CUDA_TOOLKIT_INCLUDE_DIRECTORIES = " " ${CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")
# add_definitions(-D__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__=1)

# https://developer.download.nvidia.com/video/gputechconf/gtc/2019/presentation/s9444-build-systems-exploring-modern-cmake-cuda-v2.pdf
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
#set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler=-Wall")
#set(cuda_flags "-Xcudafe=--display_error_number")

# if (WIN32)
#   add_definitions(-DNOMINMAX)
# endif()
