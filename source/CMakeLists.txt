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


# create files that contain software version etc.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/defines.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/defines.h)



# collect all source files
file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB_RECURSE CODE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE QRC_FILES ../resources/*.qrc)


# Windows application icon and version (i.e. generate windows resource files)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../resources/appversion.rc.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/appversion.rc)
if (WIN32)
  set(WINDOWS_RES_FILE ${CMAKE_CURRENT_BINARY_DIR}/appversion.obj)
  if (MSVC)
    add_custom_command(OUTPUT ${WINDOWS_RES_FILE}
      COMMAND rc.exe /fo ${WINDOWS_RES_FILE} ${CMAKE_CURRENT_BINARY_DIR}/appversion.rc
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/resources
    )
  else()
    add_custom_command(OUTPUT ${WINDOWS_RES_FILE}
      COMMAND windres.exe ${CMAKE_CURRENT_BINARY_DIR}/appversion.rc ${WINDOWS_RES_FILE}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/resources
    )
  endif()
endif()


# Configure FFMPEG
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(FFMPEG_PREFIX ${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg/win64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(FFMPEG_PREFIX ${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg/win32)
endif()
find_package(FFMPEG REQUIRED)
include_directories(${FFMPEG_INCLUDE_DIRS})


# Configure CImg
set(CIMG_PREFIX ${CMAKE_SOURCE_DIR}/3rdparty/CImg)
find_package(CIMG REQUIRED)
include_directories(${CIMG_INCLUDE_DIR})





# main executable
add_executable(${CMAKE_PROJECT_NAME} WIN32
  ${UI_HEADERS}
  ${UI_FILES}
  ${CODE_FILES}
  ${HEADER_FILES}
  ${QRC_FILES}
  ${RESOURCE_FILES}
  ${WINDOWS_RES_FILE}
)
target_link_libraries(${CMAKE_PROJECT_NAME}
  Qt5::Widgets
  ${FFMPEG_LIBRARIES}
)
add_dependencies(${CMAKE_PROJECT_NAME} SLITSCANGENERATOR_UPDATE_RANSLATIONS)



# install target
if (UNIX)
    install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
elseif (WIN32)
  install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
  install(FILES ${FFMPEG_BIN} DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()


# install system runtime lib
if( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS )
    install( PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT System )
endif( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS )

#use windeploy
if (WIN32)
    set(WINDEPLOYQTOPTION "--release")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(WINDEPLOYQTOPTION "--debug")
    endif()
    get_filename_component(CMAKE_CXX_COMPILER_BINPATH ${CMAKE_CXX_COMPILER} DIRECTORY )
    #message(STATUS "${CMAKE_CXX_COMPILER_BINPATH}")
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
            env PATH="${CMAKE_CXX_COMPILER_BINPATH}\;${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
                --dir ${CMAKE_INSTALL_PREFIX}
                --compiler-runtime
                ${WINDEPLOYQTOPTION}
                \"$<TARGET_FILE:${CMAKE_PROJECT_NAME}>\"
        COMMENT "Running windeployqt... "
    )
endif(WIN32)
