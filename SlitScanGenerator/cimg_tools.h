#ifndef CIMG_TOOLS_H
#define CIMG_TOOLS_H

#define cimg_display 0
#include "CImg.h"
#include <QImage>
#include <QString>

/** \brief convert a CImg-Image (at z-position \c == \a z) into a QImage */
QImage CImgToQImage(const cimg_library::CImg<uint8_t>& img, int z=0);

cimg_library::CImg<uint8_t> extractXZ(const cimg_library::CImg<uint8_t>& img, int y);
cimg_library::CImg<uint8_t> extractZY(const cimg_library::CImg<uint8_t>& img, int x);
cimg_library::CImg<uint8_t> extractXZ_pitch(const cimg_library::CImg<uint8_t>& img, int y,double angle);
cimg_library::CImg<uint8_t> extractZY_pitch(const cimg_library::CImg<uint8_t>& img, int x,double angle);
cimg_library::CImg<uint8_t> extractXZ_roll(const cimg_library::CImg<uint8_t>& img, int x, int y,double angle);
cimg_library::CImg<uint8_t> extractZY_roll(const cimg_library::CImg<uint8_t>& img, int x, int y,double angle);

#endif // CIMG_TOOLS_H
