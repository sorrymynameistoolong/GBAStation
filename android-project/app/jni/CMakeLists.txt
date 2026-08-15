cmake_minimum_required(VERSION 3.22.1)
project(GBASTATION_ANDROID LANGUAGES C CXX)

# libromfs produces a large generated C++ source file. Serialise native compile
# and link jobs on Android so it can build reliably alongside the emulator cores
# in constrained CI and local environments.
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    # Resource chunks and the rest of the large emulator-core graph compile
    # one at a time. This is slower but keeps dual-ABI builds within ordinary
    # CI and developer-machine memory limits.
    set_property(GLOBAL PROPERTY JOB_POOLS
        gbastation_android_native_pool=1
        gbastation_android_compile_pool=1
    )
    set(CMAKE_JOB_POOL_COMPILE gbastation_android_compile_pool)
    set(CMAKE_JOB_POOL_LINK gbastation_android_compile_pool)
endif()

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build Android native dependencies as shared libraries" FORCE)
set(SDL_SHARED ON CACHE BOOL "Build SDL as the library loaded by SDLActivity" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Avoid packaging an unused static SDL copy" FORCE)
set(SDL_TEST OFF CACHE BOOL "Disable SDL test support" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Disable SDL test executables" FORCE)

add_subdirectory(SDL)

# Borealis uses the SDL2 target name. Official SDL2 exposes it directly when
# building the shared library; keep an interface fallback for future SDL2 CMake
# export layouts that expose only the SDL2::SDL2 namespaced target.
if(NOT TARGET SDL2)
    if(TARGET SDL2::SDL2)
        add_library(SDL2 INTERFACE)
        target_link_libraries(SDL2 INTERFACE SDL2::SDL2)
    else()
        message(FATAL_ERROR "Could not find an SDL2 target after configuring SDL.")
    endif()
endif()

if(NOT TARGET SDL2main)
    add_library(SDL2main INTERFACE)
endif()

set(PLATFORM_ANDROID ON CACHE BOOL "Build GBAStation for Android" FORCE)
set(LIBROMFS_PREBUILT_GENERATOR
    "${CMAKE_CURRENT_SOURCE_DIR}/borealis/libromfs-generator"
    CACHE FILEPATH "Host libromfs resource generator" FORCE)

if(NOT EXISTS "${LIBROMFS_PREBUILT_GENERATOR}")
    message(FATAL_ERROR
        "libromfs-generator is missing at ${LIBROMFS_PREBUILT_GENERATOR}. "
        "Run ./androidbuild.sh from the repository root.")
endif()

add_subdirectory(borealis)

if(TARGET libromfs_base-GBAStation)
    # Use the same global single-job pool as the application targets so the
    # largest generated resource chunk never overlaps another C++ compile.
    set_property(TARGET libromfs_base-GBAStation PROPERTY JOB_POOL_COMPILE gbastation_android_compile_pool)
    set_property(TARGET libromfs_base-GBAStation PROPERTY JOB_POOL_LINK gbastation_android_compile_pool)
    # The generated source embeds the complete ROM filesystem as byte arrays.
    # Debug information for that data is very large and is not useful at runtime.
    target_compile_options(libromfs_base-GBAStation PRIVATE -g0)
endif()
