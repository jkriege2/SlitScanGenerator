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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/defines.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/defines.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../resources/appversion.rc.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/appversion.rc)

file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB_RECURSE CODE_FILES *.cpp)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_add_resources(RESOURCE_FILES ../resources/resources.qrc)

# Windows application icon
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

add_executable(${CMAKE_PROJECT_NAME} WIN32
  ${UI_HEADERS}
  ${CODE_FILES}
  ${RESOURCE_FILES}
  ${WINDOWS_RES_FILE}
)
target_link_libraries(${CMAKE_PROJECT_NAME}
  Qt5::Widgets
)





if (UNIX)
  install(TARGETS ${CMAKE_PROJECT_NAME}
          RUNTIME DESTINATION bin)
elseif (WIN32)
  install(TARGETS ${CMAKE_PROJECT_NAME}
          DESTINATION .)
endif()
