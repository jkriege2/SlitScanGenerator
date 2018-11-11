# Project: SlitScanGenerator (https://github.com/jkriege2/SlitScanGenerator)
# Copyright (c) 2016-2018, Jan Krieger <jan@jkrieger.de>
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

cmake_minimum_required(VERSION 2.8.11)
#set(CMAKE_VERBOSE_MAKEFILE ON)

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
  set(CMAKE_INSTALL_PREFIX dist CACHE STRING "Install path prefix.")
endif()

project(SlitScanGenerator)
set(PROJECT_LONGNAME "SlitScanGenerator")
set(PROJECT_VERSION "2018-10-05")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(FFMPEGDIR ${CMAKE_SOURCE_DIR}/ffmpeg/win64)
    set(PROJECT_BITNESS "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(FFMPEGDIR ${CMAKE_SOURCE_DIR}/ffmpeg/win32)
    set(PROJECT_BITNESS "32")
endif()

set(CIMGDIR ${CMAKE_SOURCE_DIR}/CImg)

# Global CMake options
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)


# Configure Qt
find_package(Qt5Widgets REQUIRED)

if (NOT MSVC)
  # Enable the C++11 standard
  #set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11 -O3 -fopenmp -msse2 -msse -fpmath=both)
  add_compile_options(-std=c++11)
  add_compile_options(-O3)
  add_compile_options(-fopenmp)
  add_compile_options(-msse2)
  add_compile_options(-msse)
  #add_compile_options(-fpmath=both)
  link_libraries(-lm)
  link_libraries(avutil)
  link_libraries(avcodec)
  link_libraries(avdevice)
  link_libraries(swscale)
  link_libraries(avformat)
  link_libraries(swresample)
  link_libraries(postproc)
  link_libraries(avfilter)
endif()

include_directories(${FFMPEGDIR}/include)
link_directories(${FFMPEGDIR}/lib)
include_directories(${CIMGDIR})

# Testing configuration
#enable_testing()
#set(TEST_LINK_LIBRARIES Qt5::Test)

add_subdirectory(source)

if (WIN32)
  add_subdirectory(installer_win)
endif()

