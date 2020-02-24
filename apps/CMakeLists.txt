#-------------------------------------------------------------------------
# See README.md for guidance.
#-------------------------------------------------------------------------
cmake_minimum_required ( VERSION 3.8 )

project ( tweeker_raytracer
  VERSION 2.95.0
  )

# set(CMAKE_PREFIX_PATH
#   "${PROJECT_BINARY_DIR}"
#   "${CMAKE_PREFIX_PATH}"
#   )

# message("CMAKE_PREFIX_PATH    = " "${CMAKE_PREFIX_PATH}")
# message("CMAKE_MODULE_PATH    = " "${CMAKE_MODULE_PATH}")


# find_package will search CONFIG before MODULE with this turned ON
#set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
# SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
# SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})


#ADD_SUBDIRECTORY( OptixGui )

ADD_SUBDIRECTORY( Optix7Gui )  # Uses deprecated CUDA find_package()
