# - Try to find CImg lib
#
#  CIMG_FOUND - system has CImg lib
#  CIMG_INCLUDE_DIR - the CImg include directory


macro(_cimg_check_path)

  if(EXISTS "${CIMG_INCLUDE_DIR}/CImg.h")
    set(CIMG_OK TRUE)
  endif()

  if(NOT CIMG_OK)
    message(STATUS "CImg include path was specified but no CImg.h file was found: ${CIMG_INCLUDE_DIR}")
  endif()

endmacro()

if(NOT CIMG_INCLUDE_DIR)
  message(STATUS "CImg: trying to locate CImg library")
  find_path(CIMG_INCLUDE_DIR NAMES CImg.h
	PATHS
	${CIMG_PREFIX}
	${CIMG_PREFIX}/include
	${CMAKE_INSTALL_PREFIX}/include
	${CMAKE_INSTALL_PREFIX}/include/cimg
	${CMAKE_INSTALL_PREFIX}/cimg/include
	${KDE4_INCLUDE_DIR}
  )

endif()

if(CIMG_INCLUDE_DIR)
  _cimg_check_path()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CImg DEFAULT_MSG CIMG_INCLUDE_DIR CIMG_OK)

mark_as_advanced(CIMG_INCLUDE_DIR)