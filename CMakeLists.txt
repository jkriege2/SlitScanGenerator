# Project: SlitScanGenerator (https://github.com/jkriege2/SlitScanGenerator)
# Copyright (c) 2016-2020, Jan Krieger <jan@jkrieger.de>
#
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.20)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# append path to local cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Default configuration values. These must be before the project command or
# they won't work in Windows.
# If no build type is specified, default to "Release"
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "None Debug Release RelWithDebInfo MinSizeRel"
      FORCE)
endif()
# Install to "dist" directory in Windows for testing and as a staging directory
# for the installer.
if (WIN32 AND NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/dist CACHE STRING "Install path prefix.")
endif()
message(STATUS "    creating directory: ${CMAKE_INSTALL_PREFIX}")
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX})
message(STATUS "    install prefix: ${CMAKE_INSTALL_PREFIX}")

project(SlitScanGenerator)
set(PROJECT_LONGNAME "SlitScanGenerator")
set(PROJECT_VERSION "2025.01.31")
string(TIMESTAMP PROJECT_BUILDYEAR "%Y")
string(TIMESTAMP PROJECT_BUILDDATETIME "%Y-%m-%d")
set(PROJECT_COPYRIGHT "Copyright (c) 2016-${PROJECT_BUILDYEAR} Jan Krieger jan@jkrieger.de")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(PROJECT_BITNESS "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    error("32-bit builds are not supported")
endif()


option(UPDATE_TRANSLATIONS "Call lupdate to update translation files from source." ON)
option(TRANS_KEEP_OBSOLETE "Add -noobsolete option to lupdate command to get rid of old text entries" OFF)
option(ANALYZE_TIMING "if activate, additional code is compiled in, which outputs timing info on the console (sets macro ANALYZE_TIMING)" OFF)


# Global CMake options
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set high optimization flags for different compilers
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /DNDEBUG")
endif()

# Configure Qt
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets  LinguistTools REQUIRED)
# Retrieve the absolute path to qmake and then use that path to find
# the binaries
get_target_property(_qmake_executable Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

if (ANALYZE_TIMING)
    add_definitions(-DANALYZE_TIMING)
endif(ANALYZE_TIMING)


# Testing configuration
#enable_testing()
#set(TEST_LINK_LIBRARIES Qt${QT_VERSION_MAJOR}::Test)

add_subdirectory(source)
add_subdirectory(translations)


# PREPARE install of MinGW system libraries
#if( MINGW )
#    message( STATUS "    Installing system-libraries: MinGW DLLs." )
#    get_filename_component( Mingw_Path ${CMAKE_CXX_COMPILER} PATH )
#    set( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${Mingw_Path}/mingwm10.dll ${Mingw_Path}/libgcc_s_dw2-1.dll ${Mingw_Path}/libstdc++-6.dll )
#endif( MINGW )
include( InstallRequiredSystemLibraries )

if (WIN32)
  add_subdirectory(installer_win)
endif()

