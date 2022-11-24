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


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# collect all source files
#qt_wrap_ui(${UI_FILES})

set(CODE_FILES
        videoreader.cpp
        aboutbox.cpp
        cimg_tools.cpp
        ffmpeg_tools.cpp
        geo_tools.cpp
        imageviewer.cpp
        importdialog.cpp
        mainwindow.cpp
        optionsdialog.cpp
        processingparametertable.cpp
        processingtask.cpp
        processingthread.cpp
        processingwidget.cpp
        slitscangeneratorsettings.cpp
        taskswidget.cpp
        videoreader_ffmpeg.cpp
        videoreader_cimg.cpp
        main.cpp
        videopreviewreaderthread.cpp
        cpp_tools.cpp
        qt_tools.cpp
        imagewriter.cpp
        imagewriter_png.cpp
        imagewriter_cimg.cpp
        processingtaskreporter.cpp
        processingtaskreporter_dummy.cpp
        configio.cpp
        configio_ini.cpp
        configio_dummy.cpp
    )
set(HEADER_FILES
        videoreader.h
        aboutbox.h
        cimg_tools.h
        ffmpeg_tools.h
        geo_tools.h
        imageviewer.h
        importdialog.h
        mainwindow.h
        optionsdialog.h
        processingparametertable.h
        processingtask.h
        processingthread.h
        processingwidget.h
        slitscangeneratorsettings.h
        taskswidget.h
        videoreader_ffmpeg.h
        videoreader_cimg.h
        videopreviewreaderthread.h
        cpp_tools.h
        qt_tools.h
        imagewriter.h
        imagewriter_png.h
        imagewriter_cimg.h
        processingtaskreporter.h
        processingtaskreporter_dummy.h
        configio.h
        configio_ini.h
        configio_dummy.h
    )
set(UI_FILES
        aboutbox.ui
        importdialog.ui
        mainwindow.ui
        optionsdialog.ui
        processingwidget.ui
    )
set(QRC_FILES
        ../resources/resources.qrc
   )

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
  Qt${QT_VERSION_MAJOR}::Widgets
  ${FFMPEG_LIBRARIES}
)
add_dependencies(${CMAKE_PROJECT_NAME} SLITSCANGENERATOR_UPDATE_RANSLATIONS)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})



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


include(jk_deployqt)
jk_deployqt(${CMAKE_PROJECT_NAME})
