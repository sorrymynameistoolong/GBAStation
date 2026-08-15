cmake_minimum_required(VERSION 3.16)

option(LIBROMFS_PROJECT_NAME "Project name" "")
option(LIBROMFS_RESOURCE_LOCATION "Resource location" "")
option(LIBROMFS_PREBUILT_GENERATOR "Using prebuilt resources generator" "")

if (NOT LIBROMFS_PROJECT_NAME)
    message(FATAL_ERROR "LIBROMFS_PROJECT_NAME is not set")
endif ()

if (NOT LIBROMFS_RESOURCE_LOCATION)
    message(FATAL_ERROR "LIBROMFS_RESOURCE_LOCATION is not set")
endif ()

project(libromfs_base-${LIBROMFS_PROJECT_NAME})
set(ROMFS "libromfs_resources.cpp")

# Make sure libromfs is configured correctly
if (NOT DEFINED LIBROMFS_RESOURCE_LOCATION)
    message(FATAL_ERROR "To use libromfs, please point the variable LIBROMFS_RESOURCE_LOCATION to a valid folder path")
elseif (NOT EXISTS ${LIBROMFS_RESOURCE_LOCATION} OR NOT IS_DIRECTORY ${LIBROMFS_RESOURCE_LOCATION})
    message(FATAL_ERROR "LIBROMFS_RESOURCE_LOCATION has been defined but does not point to a valid folder")
endif ()

# The nested lib directory creates its library with this CMake project's
# name. Export the actual target so downstream consumers receive its headers
# and do not fall back to a non-existent -l library name.
set(LIBROMFS_LIBRARY ${PROJECT_NAME} PARENT_SCOPE)

# Add libromfs components
if(NOT LIBROMFS_PREBUILT_GENERATOR)
    add_subdirectory(generator)
endif ()
add_subdirectory(lib)

