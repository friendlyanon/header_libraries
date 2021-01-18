# Copyright (c) Darrell Wright
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/beached/header_libraries
#

cmake_minimum_required(VERSION 3.14) 

project(
    header_libraries
    VERSION 1.5.0
    DESCRIPTION "Various headers"
    HOMEPAGE_URL "https://github.com/beached/header_libraries"
    LANGUAGES CXX
)

set(daw_header_libraries_warning_guard SYSTEM)
if(daw_header_libraries_INCLUDE_WITHOUT_SYSTEM)
  set(daw_header_libraries_warning_guard "")
endif()

add_library(header_libraries INTERFACE) 
add_library(daw::header_libraries ALIAS header_libraries)

target_compile_features(header_libraries INTERFACE cxx_std_17)
target_include_directories(
    header_libraries
    ${daw_header_libraries_warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(header_libraries_directory "header_libraries-${PROJECT_VERSION}")
set(
    header_libraries_include_directory
    "${CMAKE_INSTALL_INCLUDEDIR}/${header_libraries_directory}"
)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "${header_libraries_include_directory}"
    COMPONENT header_libraries_Development
)

install(
  TARGETS header_libraries
	EXPORT header_librariesTargets
  INCLUDES DESTINATION "${header_libraries_include_directory}"
)

write_basic_package_version_file(
    header_librariesConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

set(
    header_libraries_install_cmakedir
    "${CMAKE_INSTALL_LIBDIR}/cmake/${header_libraries_directory}"
)

install(
    FILES "${PROJECT_BINARY_DIR}/header_librariesConfigVersion.cmake"
    DESTINATION "${header_libraries_install_cmakedir}"
    COMPONENT header_libraries_Development
)

install(
    EXPORT header_librariesTargets
    FILE header_librariesConfig.cmake
    NAMESPACE daw::
    DESTINATION "${header_libraries_install_cmakedir}"
    COMPONENT header_libraries_Development
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  include(CPack)
endif()
