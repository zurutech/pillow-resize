cmake_minimum_required(VERSION 3.1)
project(
  PillowResize
  VERSION 1.0.0
  LANGUAGES CXX)

#
# BUILD SETTINGS
#

# Set compilation flags
if(NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING "" FORCE)
  endif()

endif()

# (un)set: cmake -DCVV_DEBUG_MODE=OFF ..
option(CVV_DEBUG_MODE "cvvisual-debug-mode" OFF)
if(CVV_DEBUG_MODE MATCHES ON)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCVVISUAL_DEBUGMODE")
endif()

# Clang tidy setup: add option for enabling clang tidy checks and search for the
# exe
option(ENABLE_CLANG_TIDY
       "Output clang-tidy suggestions during the building process" OFF)
find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable")

if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

#
# BUILD DEPENDENCIES
#
# Build and setup the correct cmake variables for third-party libraries
#

# Find required libraries
find_package(OpenCV REQUIRED)

# the CLI11 library
if(NOT TARGET CLI11::CLI11)
  add_subdirectory("${PROJECT_SOURCE_DIR}/libs/CLI11/")
endif()

# the magic_enum  library
if(NOT TARGET magic_enum::magic_enum)
  add_subdirectory("${PROJECT_SOURCE_DIR}/libs/magic_enum/")
endif()

# updated in the following steps
set(RESIZE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include/PillowResize")

#
# Build project
#
add_subdirectory(src/PillowResize)

# the variable for the PillowResize libraries
set(PILLOWRESIZE_LIBS PillowResize)

#
# Add executable
#
add_subdirectory(bin)

#
# Installation support
#
message(STATUS "INSTALLATION SUPPORT")

set(PILLOWRESIZE_INSTALLING PillowResize)

# Specify the installation configuration directory
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Installing Targets
# (https://cmake.org/cmake/help/latest/command/install.html#installing-targets)
install(
  TARGETS ${PILLOWRESIZE_INSTALLING}
  EXPORT pillowresize-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Installing Directories
# (https://cmake.org/cmake/help/latest/command/install.html#installing-directories)
# The DIRECTORY form installs contents of one or more directories to a given
# destination. Here we are installing the contents of the include directory
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Installing Exports
# https://cmake.org/cmake/help/latest/command/install.html#installing-exports
# The EXPORT form generates and installs a CMake file containing code to import
# targets from the installation tree into another project. Target installations
# are associated with the export <export-name> using the EXPORT option of the
# install(TARGETS) FILE does not specify a source but the name of the generated
# file (default: <export-name>.cmake)
install(
  EXPORT pillowresize-targets
  FILE ${PROJECT_NAME}Targets.cmake
  DESTINATION ${INSTALL_CONFIGDIR})

# Helpers functions for creating config files that can be included by other
# projects to find and use a package. Adds the configure_package_config_file()
# and write_basic_package_version_file() commands.
include(CMakePackageConfigHelpers)

# Create a ConfigVersion.cmake file
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

# configure_package_config_file() should be used instead of the plain
# configure_file() command when creating the <PackageName>Config.cmake or
# <PackageName>config.cmake file for installing a project or library. It helps
# making the resulting package relocatable by avoiding hardcoded paths in the
# installed Config.cmake file
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

# Install the config, configversion and custom find modules
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${INSTALL_CONFIGDIR})

# ##############################################################################

export(EXPORT pillowresize-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)

# Register package in user's package registry
export(PACKAGE ${PROJECT_NAME})

# ##############################################################################

# Add support for uninstall target As seen on
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

  add_custom_target(
    uninstall COMMAND ${CMAKE_COMMAND} -P
                      ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
