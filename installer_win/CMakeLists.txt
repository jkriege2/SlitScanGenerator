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



# FFMPEG
file(GLOB FFMPEG_ZIPS
    ${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg/*.zip
)
install(FILES ${FFMPEG_ZIPS}
  DESTINATION 3rdparty/ffmpeg
)

# CImg
file(GLOB CIMG_ZIPS
    ${CMAKE_SOURCE_DIR}/3rdparty/CImg/*.zip
)
install(FILES ${CIMG_ZIPS}
  DESTINATION 3rdparty/CImg
)


# 3rdParty Readmy
install(FILES ${CMAKE_SOURCE_DIR}/3rdparty/CImg/readme.txt
  DESTINATION 3rdparty
)

# Examples
install(DIRECTORY
  ${CMAKE_SOURCE_DIR}/testmovie/
  DESTINATION testmovie
)
