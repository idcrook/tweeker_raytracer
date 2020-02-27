#-------------------------------------------------------------------------
# See README.md for guidance.
#-------------------------------------------------------------------------
cmake_minimum_required ( VERSION 3.8 )

project ( tweeker_raytracer
  VERSION 2.95.0
  )

# find_package will search CONFIG before MODULE with this turned ON
#set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")


ADD_SUBDIRECTORY( apps/OptixGui )

ADD_SUBDIRECTORY( apps/Optix7Gui )

ADD_SUBDIRECTORY( apps/rtigo3 )
