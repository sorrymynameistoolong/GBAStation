set(CMAKE_CXX_STANDARD 20)

# Gather romfs files.
file(GLOB_RECURSE ROMFS_FILES
    "${LIBROMFS_RESOURCE_LOCATION}/*"
)

set(ROMFS_MAIN "${CMAKE_CURRENT_BINARY_DIR}/libromfs_resources.cpp")
set(ROMFS_GENERATED_SOURCES "${ROMFS_MAIN}")

# The Android build invokes a host-side generator before CMake configures the
# native target. Generate and discover one source per resource at configure
# time so CMake never needs to compile a single huge byte-array translation
# unit for the whole ROM filesystem.
if (LIBROMFS_PREBUILT_GENERATOR)
    file(GLOB _OLD_ROMFS_CHUNKS "${CMAKE_CURRENT_BINARY_DIR}/libromfs_resource_*.cpp")
    if (_OLD_ROMFS_CHUNKS)
        file(REMOVE ${_OLD_ROMFS_CHUNKS})
    endif()
    execute_process(
        COMMAND ${LIBROMFS_PREBUILT_GENERATOR}
                ${LIBROMFS_PROJECT_NAME} ${LIBROMFS_RESOURCE_LOCATION}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RESULT_VARIABLE _romfs_generation_result
    )
    if (NOT _romfs_generation_result EQUAL 0)
        message(FATAL_ERROR "Failed to generate libromfs sources (exit ${_romfs_generation_result})")
    endif()
    file(GLOB ROMFS_GENERATED_SOURCES
        "${CMAKE_CURRENT_BINARY_DIR}/libromfs_resources.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/libromfs_resource_*.cpp"
    )
endif()

# Add sources.
add_library(${PROJECT_NAME} STATIC
    ${ROMFS_GENERATED_SOURCES}
    source/romfs.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC include)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_compile_definitions(${PROJECT_NAME} PUBLIC LIBROMFS_PROJECT_NAME=${LIBROMFS_PROJECT_NAME})

if (USE_BOOST_FILESYSTEM)
    find_package(Boost 1.44 REQUIRED COMPONENTS filesystem)
    if(Boost_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE Boost::filesystem)
        target_compile_options(${PROJECT_NAME} PRIVATE -DUSE_BOOST_FILESYSTEM)
    endif()
endif ()

# Regenerate sources when resource data changes. The main source is the stable
# custom-command output; chunk files are re-discovered on the next configure.
if (LIBROMFS_PREBUILT_GENERATOR)
    message(STATUS "Using prebuilt libromfs-generator: ${LIBROMFS_PREBUILT_GENERATOR}")
    add_custom_command(OUTPUT ${ROMFS_MAIN}
        COMMAND ${LIBROMFS_PREBUILT_GENERATOR}
            ${LIBROMFS_PROJECT_NAME} ${LIBROMFS_RESOURCE_LOCATION}
        DEPENDS ${ROMFS_FILES}
    )
else ()
    message(STATUS "Using libromfs-generator: $<TARGET_FILE:libromfs-generator>")
    add_custom_command(OUTPUT ${ROMFS_MAIN}
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:libromfs-generator>
            ${LIBROMFS_PROJECT_NAME} ${LIBROMFS_RESOURCE_LOCATION}
        DEPENDS ../generator ${ROMFS_FILES}
    )
endif ()

add_custom_target(romfs_file_packer-${LIBROMFS_PROJECT_NAME} ALL DEPENDS ${ROMFS_FILES})
