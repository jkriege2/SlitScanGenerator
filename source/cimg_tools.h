#ifndef CIMG_TOOLS_H
#define CIMG_TOOLS_H

#define _USE_MATH_DEFINES
#include "CImg.h"
#include <QImage>
#include <QString>

inline size_t qHash(const QPointF &key, size_t seed = 0) {
    return qHash(key.x()+key.y(), seed);
}

inline size_t qHash(const QPoint &key, size_t seed = 0) {
    return qHash(key.x()+key.y(), seed);
}

namespace std {
    template<>
    struct hash<QPointF>
    {
        std::size_t operator()(const QPointF& s) const noexcept
        {
            std::size_t h1 = std::hash<double>{}(s.x());
            std::size_t h2 = std::hash<double>{}(s.y());
            return h1 ^ (h2 << 1); // or use boost::hash_combine
        }
    };
}

/** \brief convert a CImg-Image (at z-position \c == \a z) into a QImage */
QImage CImgToQImage(const cimg_library::CImg<uint8_t>& img, int z=0);

typedef std::function<float(const cimg_library::CImg<uint8_t>& img, const float fx, const float fy, const int z, const int c)> interpolatingAtXYFunctor;

cimg_library::CImg<uint8_t> extractXZ_atz(int z, const cimg_library::CImg<uint8_t>& img, int y);
cimg_library::CImg<uint8_t> extractZY_atz(int z, const cimg_library::CImg<uint8_t>& img, int x);
cimg_library::CImg<uint8_t> extractXZ_atz_pitch(int z, int depth, const cimg_library::CImg<uint8_t>& img, int y,double angle, const interpolatingAtXYFunctor& atFunc, int& zout, int* lenout=nullptr);
cimg_library::CImg<uint8_t> extractZY_atz_pitch(int z, int depth, const cimg_library::CImg<uint8_t>& img, int x,double angle, const interpolatingAtXYFunctor& atFunc, int& zout, int* lenout=nullptr);
cimg_library::CImg<uint8_t> extractXZ_atz_roll(int z, int depth, const cimg_library::CImg<uint8_t>& img, int x, int y,double angle, const interpolatingAtXYFunctor& atFunc);
cimg_library::CImg<uint8_t> extractZY_atz_roll(int z, int depth, const cimg_library::CImg<uint8_t>& img, int x, int y,double angle, const interpolatingAtXYFunctor& atFunc);


#endif // CIMG_TOOLS_H
