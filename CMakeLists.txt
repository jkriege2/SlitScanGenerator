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

cmake_minimum_required(VERSION 3.11)
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
set(PROJECT_VERSION "2020-04")
string(TIMESTAMP PROJECT_BUILDYEAR "%Y")
string(TIMESTAMP PROJECT_BUILDDATETIME "%Y-%m-%d")
set(PROJECT_COPYRIGHT "Copyright (c) 2016-${PROJECT_BUILDYEAR} Jan Krieger jan@jkrieger.de")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(PROJECT_BITNESS "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(PROJECT_BITNESS "32")
endif()




# Global CMake options
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure Qt
find_package(Qt5Widgets REQUIRED)
# Retrieve the absolute path to qmake and then use that path to find
# the binaries
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")


# Testing configuration
#enable_testing()
#set(TEST_LINK_LIBRARIES Qt5::Test)

add_subdirectory(source)


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

