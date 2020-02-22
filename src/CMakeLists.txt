#-------------------------------------------------------------------------
# See README.md for guidance.
#-------------------------------------------------------------------------
cmake_minimum_required ( VERSION 3.11 )

project ( tweeker_raytracer
  VERSION 2.95.0
  )

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# push CMake configs to each subdirectory
ADD_SUBDIRECTORY( OptixGui )

ADD_SUBDIRECTORY( Optix7Gui )
