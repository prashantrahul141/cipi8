cmake_minimum_required(VERSION 3.22)

# default build type will be Debug, if nothing is specified.
set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")

  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# project name.
project(cipi8)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# versions
set(SDL_VERSION "2.30.7")

# Add FetchContent module
include(FetchContent)

# FetchContent: Raylib 
FetchContent_Declare(
  SDL2
  URL "https://github.com/libsdl-org/SDL/archive/refs/tags/release-${SDL_VERSION}.zip"
)

# download and local install 
FetchContent_MakeAvailable(SDL2)

# All source files.
file(GLOB SOURCES src/external/nhlog.c src/platform.cpp src/chip8.cpp src/app.cpp src/main.cpp)
set_source_files_properties(src/external/nhlog.c PROPERTIES LANGUAGE CXX)

# include dir
include_directories(${PROJECT_NAME} PRIVATE src/)

# macros
add_compile_definitions(LOG_USE_COLOR)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})


# compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # msvc related flags

  set(MSVC_COMPILE_OPTIONS "/W4;")
  set(MSVC_COMPILE_OPTIONS_DEBUG "${MSVC_COMPILE_OPTIONS};/DCIPI8_DEBUG_MODE;")
  set(MSVC_COMPILE_OPTIONS_RELEASE "${MSVC_COMPILE_OPTIONS};/O2;")

  # debug
  target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${MSVC_COMPILE_OPTIONS_DEBUG}>")

  # release
  target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${MSVC_COMPILE_OPTIONS_RELEASE}>")

else()
  # gcc and clang 
  set(GCC_COMPILE_OPTIONS "-Wall;-Wextra;-Wpedantic")
  set(GCC_COMPILE_OPTIONS_DEBUG "${GCC_COMPILE_OPTIONS};-DCIPI8_DEBUG_MODE;-ggdb;-g;")
  set(GCC_COMPILE_OPTIONS_RELEASE "${GCC_COMPILE_OPTIONS};-O3;")

  # debug
  target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${GCC_COMPILE_OPTIONS_DEBUG}>")

  # release
  target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${GCC_COMPILE_OPTIONS_RELEASE}>")

  # Linker flags.
  # set(GCC_LINK_OPTIONS_RELEASE "-s;-static;")

  target_link_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${GCC_LINK_OPTIONS_RELEASE}>")

endif()


# linking libs
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2)

