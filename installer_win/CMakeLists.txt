# Project: JKHouseholdAccounts (https://github.com/jkriege2/JKHouseholdAccounts)
# Copyright (c) 2018, Jan Krieger <jan@jkrieger.de>
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

# Configure the installer scripts
configure_file(installer.cmake.nsi ${CMAKE_BINARY_DIR}/installer.nsi @ONLY)

# Compiler Runtime DLLs
if (MSVC)
  # Visual Studio
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP true)
  include(InstallRequiredSystemLibraries)
  install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION .)
else()
  # MinGW
  get_filename_component(MINGW_DLL_DIR ${CMAKE_CXX_COMPILER} PATH)
  message(${MINGW_DLL_DIR})
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      # 64 bits
      install(FILES
          ${MINGW_DLL_DIR}/libgcc_s_seh-1.dll
          ${MINGW_DLL_DIR}/libbz2-1.dll
          ${MINGW_DLL_DIR}/libfreetype-6.dll
          ${MINGW_DLL_DIR}/libglib-2.0-0.dll
          ${MINGW_DLL_DIR}/libgraphite2.dll
          ${MINGW_DLL_DIR}/libharfbuzz-0.dll
          ${MINGW_DLL_DIR}/libiconv-2.dll
          ${MINGW_DLL_DIR}/libicudt57.dll
          ${MINGW_DLL_DIR}/libicuin57.dll
          ${MINGW_DLL_DIR}/libicuio57.dll
          ${MINGW_DLL_DIR}/libicule57.dll
          ${MINGW_DLL_DIR}/libiculx57.dll
          ${MINGW_DLL_DIR}/libicutest57.dll
          ${MINGW_DLL_DIR}/libicutu57.dll
          ${MINGW_DLL_DIR}/libicuuc57.dll
          ${MINGW_DLL_DIR}/libintl-8.dll
          ${MINGW_DLL_DIR}/libjasper-1.dll
          ${MINGW_DLL_DIR}/libjpeg-8.dll
          ${MINGW_DLL_DIR}/liblzma-5.dll
          ${MINGW_DLL_DIR}/libpcre-1.dll
          ${MINGW_DLL_DIR}/libpcre16-0.dll
          ${MINGW_DLL_DIR}/libpng16-16.dll
          ${MINGW_DLL_DIR}/libtiff-5.dll
          ${MINGW_DLL_DIR}/libtiffxx-5.dll
          ${MINGW_DLL_DIR}/zlib1.dll
        DESTINATION .
      )
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
      # 32 bits
      install(FILES
        ${MINGW_DLL_DIR}/libgcc_s_dw2-1.dll
        DESTINATION .
      )
  endif()

  install(FILES
    ${MINGW_DLL_DIR}/libatomic-1.dll
    ${MINGW_DLL_DIR}/libstdc++-6.dll
    ${MINGW_DLL_DIR}/libwinpthread-1.dll
    DESTINATION .
  )



endif()

# Qt Libraries
get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)
foreach(plugin ${Qt5Gui_PLUGINS})
  get_target_property(QT_PLUGIN_DIR ${plugin} LOCATION)
  #message(${QT_PLUGIN_DIR})
endforeach()
get_filename_component(QT_PLUGIN_DIR ${QT_PLUGIN_DIR} PATH)
#message(${QT_PLUGIN_DIR})
set(QT_PLUGIN_DIR ${QT_PLUGIN_DIR}/../)
#message(${QT_PLUGIN_DIR})


install(FILES
#  ${QT_DLL_DIR}/icudt51.dll
#  ${QT_DLL_DIR}/icuin51.dll
#  ${QT_DLL_DIR}/icuuc51.dll
  ${QT_DLL_DIR}/Qt5Core.dll
  ${QT_DLL_DIR}/Qt5Gui.dll
  ${QT_DLL_DIR}/Qt5Widgets.dll
  ${CMAKE_SOURCE_DIR}/installer_win/qt.conf
  ${CMAKE_SOURCE_DIR}/LICENSE.txt
  DESTINATION .
)
# Qt Platform Plugin
install(FILES
  ${QT_PLUGIN_DIR}/platforms/qwindows.dll
  DESTINATION plugins/platforms
)
# Qt imageformats Plugin
install(FILES
  ${QT_PLUGIN_DIR}/imageformats/qjpeg.dll
  ${QT_PLUGIN_DIR}/imageformats/qtiff.dll
  ${QT_PLUGIN_DIR}/imageformats/qgif.dll
  ${QT_PLUGIN_DIR}/imageformats/qwebp.dll
  ${QT_PLUGIN_DIR}/imageformats/qwbmp.dll
  ${QT_PLUGIN_DIR}/imageformats/qtga.dll
  ${QT_PLUGIN_DIR}/imageformats/qsvg.dll
  ${QT_PLUGIN_DIR}/imageformats/qico.dll
  ${QT_PLUGIN_DIR}/imageformats/qicns.dll
  DESTINATION plugins/imageformats
)
# FFMPEG/bin
install(DIRECTORY ${FFMPEGDIR}/bin/
  DESTINATION .
  FILES_MATCHING PATTERN "*.dll"
)
# FFMPEG
install(DIRECTORY
  ${FFMPEGDIR}/
  DESTINATION 3rdparty/ffmpeg
)
# CImg
install(DIRECTORY
  ${CIMGDIR}/
  DESTINATION 3rdparty/CImg
)

# Examples
install(DIRECTORY
  ${CMAKE_SOURCE_DIR}/testmovie/
  DESTINATION testmovie
)
