cmake_minimum_required(VERSION 3.16)
project(FreeGLUT_Template VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

# ============================================================================
# Platform Detection
# ============================================================================
if(APPLE)
    message(STATUS "Building on macOS")
    set(IS_MACOS TRUE)
elseif(UNIX)
    message(STATUS "Building on Linux")
    set(IS_LINUX TRUE)
endif()

# ============================================================================
# Options
# ============================================================================
# On macOS, use native GLUT framework by default (more stable)
# On Linux, use FreeGLUT by default
if(IS_MACOS)
    option(USE_FREEGLUT "Use FreeGLUT instead of system GLUT" OFF)
else()
    option(USE_FREEGLUT "Use FreeGLUT instead of system GLUT" ON)
endif()
option(BUILD_EXAMPLES "Build example programs" ON)
option(USE_XQUARTZ "Use XQuartz/X11 instead of native macOS FreeGLUT" OFF)

# ============================================================================
# macOS Specific Configuration (MUST be before find_package)
# ============================================================================
if(IS_MACOS)
    # Find Homebrew prefix and set paths BEFORE find_package calls
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE BREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(BREW_PREFIX)
        message(STATUS "Homebrew prefix: ${BREW_PREFIX}")
        list(APPEND CMAKE_PREFIX_PATH "${BREW_PREFIX}")
        list(APPEND CMAKE_LIBRARY_PATH "${BREW_PREFIX}/lib")
        list(APPEND CMAKE_INCLUDE_PATH "${BREW_PREFIX}/include")
        set(CMAKE_PREFIX_PATH "${BREW_PREFIX};${CMAKE_PREFIX_PATH}")
    endif()

    # XQuartz/X11 paths (optional - only if USE_XQUARTZ is ON)
    # NOTE: By default, macOS uses native FreeGLUT without X11
    #       Enable USE_XQUARTZ=ON only if you specifically need X11 support
    if(USE_XQUARTZ)
        set(XQUARTZ_PATH "/opt/X11")
        if(EXISTS ${XQUARTZ_PATH})
            message(STATUS "Using XQuartz/X11 (USE_XQUARTZ=ON)")
            message(STATUS "Found XQuartz at ${XQUARTZ_PATH}")
            list(APPEND CMAKE_LIBRARY_PATH "${XQUARTZ_PATH}/lib")
            list(APPEND CMAKE_INCLUDE_PATH "${XQUARTZ_PATH}/include")
        else()
            message(WARNING "USE_XQUARTZ is ON but XQuartz not found at ${XQUARTZ_PATH}")
            message(STATUS "Install XQuartz: brew install --cask xquartz")
        endif()
    else()
        message(STATUS "Using native macOS FreeGLUT (no X11)")
    endif()
endif()

# ============================================================================
# Find OpenGL
# ============================================================================
find_package(OpenGL REQUIRED)

# ============================================================================
# Find GLEW
# ============================================================================
find_package(GLEW QUIET)

if(TARGET GLEW::GLEW)
    message(STATUS "Found GLEW (GLEW::GLEW target)")
    set(GLEW_TARGET GLEW::GLEW)
    set(GLEW_FOUND TRUE)
elseif(TARGET glew)
    message(STATUS "Found GLEW (glew target)")
    set(GLEW_TARGET glew)
    set(GLEW_FOUND TRUE)
else()
    if(IS_LINUX)
        find_package(PkgConfig)
        if(PkgConfig_FOUND)
            pkg_check_modules(GLEW glew)
            if(GLEW_FOUND)
                message(STATUS "Found GLEW via pkg-config")
                set(GLEW_TARGET ${GLEW_LIBRARIES})
            endif()
        endif()
    endif()

    if(NOT GLEW_FOUND)
        find_path(GLEW_INCLUDE_DIR GL/glew.h)
        find_library(GLEW_LIBRARY NAMES GLEW glew libglew)
        if(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
            message(STATUS "Found GLEW (manual search)")
            set(GLEW_TARGET ${GLEW_LIBRARY})
            set(GLEW_FOUND TRUE)
        endif()
    endif()
endif()

if(NOT GLEW_FOUND)
    message(WARNING "GLEW not found. Building without GLEW support.\n"
        "Install GLEW:\n"
        "  macOS: brew install glew\n"
        "  Linux: sudo apt-get install libglew-dev")
    set(GLEW_TARGET "")
endif()

# ============================================================================
# Find GLUT / FreeGLUT
# ============================================================================
set(GLUT_FOUND FALSE)

if(IS_LINUX)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(FREEGLUT freeglut)
        if(FREEGLUT_FOUND)
            message(STATUS "Found FreeGLUT via pkg-config")
            set(GLUT_TARGET ${FREEGLUT_LIBRARIES})
            set(GLUT_INCLUDE ${FREEGLUT_INCLUDE_DIRS})
            set(GLUT_FOUND TRUE)
        else()
            find_package(GLUT REQUIRED)
            if(TARGET GLUT::GLUT)
                message(STATUS "Using system GLUT (GLUT::GLUT)")
                set(GLUT_TARGET GLUT::GLUT)
            else()
                message(STATUS "Using system GLUT (legacy)")
                set(GLUT_TARGET GLUT)
            endif()
            set(GLUT_FOUND TRUE)
        endif()
    endif()
elseif(USE_FREEGLUT)
    find_package(FreeGLUT QUIET)

    if(TARGET FreeGLUT::freeglut)
        message(STATUS "Found FreeGLUT (FreeGLUT::freeglut)")
        set(GLUT_TARGET FreeGLUT::freeglut)
        set(GLUT_FOUND TRUE)
    elseif(TARGET freeglut::freeglut)
        message(STATUS "Found FreeGLUT (freeglut::freeglut)")
        set(GLUT_TARGET freeglut::freeglut)
        set(GLUT_FOUND TRUE)
    else()
        find_package(GLUT REQUIRED)
        if(TARGET GLUT::GLUT)
            message(STATUS "Using system GLUT (GLUT::GLUT)")
            set(GLUT_TARGET GLUT::GLUT)
        else()
            message(STATUS "Using system GLUT (legacy)")
            set(GLUT_TARGET GLUT)
        endif()
        set(GLUT_FOUND TRUE)
    endif()
else()
    find_package(GLUT REQUIRED)
    if(TARGET GLUT::GLUT)
        set(GLUT_TARGET GLUT::GLUT)
    else()
        set(GLUT_TARGET GLUT)
    endif()
endif()

# ============================================================================
# Source Files
# ============================================================================
set(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/src/common.h
)

set(SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

# ============================================================================
# Executable
# ============================================================================
set(TARGET_NAME "FreeGLUT_Template")

add_executable(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${OPENGL_INCLUDE_DIR}
    ${GLUT_INCLUDE}
)

# Define USE_FREEGLUT if using FreeGLUT
if(USE_FREEGLUT AND (TARGET FreeGLUT::freeglut OR TARGET freeglut::freeglut))
    target_compile_definitions(${TARGET_NAME} PRIVATE USE_FREEGLUT)
endif()

# Define USE_GLEW if GLEW was found
if(GLEW_FOUND)
    target_compile_definitions(${TARGET_NAME} PRIVATE USE_GLEW)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_TARGET}
    ${GLEW_TARGET}
)

# Platform specific libraries
if(IS_MACOS)
    target_link_libraries(${TARGET_NAME} PRIVATE
        "-framework OpenGL"
    )
elseif(IS_LINUX)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET_NAME} PRIVATE
        Threads::Threads
        m
    )
endif()

# Set properties
set_target_properties(${TARGET_NAME} PROPERTIES
    OUTPUT_NAME ${TARGET_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output
)

# ============================================================================
# Examples (Optional)
# ============================================================================
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ============================================================================
# Generate Scripts from Templates
# ============================================================================
set(BUILD_DIR_NAME "build")
set(OUTPUT_DIR_NAME "output")

configure_file(
    ${CMAKE_SOURCE_DIR}/scripts/build.sh.in
    ${CMAKE_SOURCE_DIR}/scripts/build.sh
    @ONLY
)

configure_file(
    ${CMAKE_SOURCE_DIR}/scripts/clean.sh.in
    ${CMAKE_SOURCE_DIR}/scripts/clean.sh
    @ONLY
)

configure_file(
    ${CMAKE_SOURCE_DIR}/scripts/run.sh.in
    ${CMAKE_SOURCE_DIR}/scripts/run.sh
    @ONLY
)

# Make scripts executable
execute_process(COMMAND chmod +x ${CMAKE_SOURCE_DIR}/scripts/build.sh)
execute_process(COMMAND chmod +x ${CMAKE_SOURCE_DIR}/scripts/clean.sh)
execute_process(COMMAND chmod +x ${CMAKE_SOURCE_DIR}/scripts/run.sh)

# ============================================================================
# Clean Project Target
# ============================================================================
add_custom_target(clean-project
    COMMENT "Cleaning all generated files (build, output, scripts)..."
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/output
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/scripts/build.sh
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/scripts/clean.sh
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/scripts/run.sh
    COMMAND ${CMAKE_COMMAND} -E echo "Clean complete!"
)

# ============================================================================
# Installation
# ============================================================================
install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
)

# ============================================================================
# Configuration Summary
# ============================================================================
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  OpenGL: ${OPENGL_FOUND}")
message(STATUS "  GLUT Target: ${GLUT_TARGET}")
message(STATUS "  GLEW: ${GLEW_FOUND}")
message(STATUS "  GLEW Target: ${GLEW_TARGET}")
if(IS_MACOS)
    message(STATUS "  XQuartz/X11: ${USE_XQUARTZ}")
endif()
message(STATUS "  Output Directory: ${CMAKE_SOURCE_DIR}/output")
message(STATUS "")
