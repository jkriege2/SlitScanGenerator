# Project: SlitScanGenerator (https://github.com/jkriege2/SlitScanGenerator)
# Copyright (c) 2016-2026, Jan Krieger <jan@jkrieger.de>
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



# Third-party licenses from vcpkg packages.
install(FILES
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/cimg/copyright"
  DESTINATION 3rdparty/CImg
)
install(FILES
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/ffmpeg/copyright"
  DESTINATION 3rdparty/ffmpeg
)


# Examples
install(DIRECTORY
  ${CMAKE_SOURCE_DIR}/testmovie/
  DESTINATION testmovie
)
