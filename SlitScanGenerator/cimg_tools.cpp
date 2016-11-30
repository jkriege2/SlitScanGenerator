#include "cimg_tools.h"


QImage CImgToQImage(const cimg_library::CImg<uint8_t> &img, int z)
{
    QImage imgQt(img.width(), img.height(), QImage::Format_ARGB32);

    cimg_forXY( img, x, y )
    {
        const uint8_t R = img( x, y, z, 0 );
        const uint8_t G = img( x, y, z, 1 );
        const uint8_t B = img( x, y, z, 2 );
        imgQt.setPixel( x, y, qRgb(R,G,B) );
    }

    return imgQt;
}



cimg_library::CImg<uint8_t> extractXZ(const cimg_library::CImg<uint8_t> &img_src, int y)
{
    cimg_library::CImg<uint8_t> img(img_src.width(), img_src.depth(),1,3);
    cimg_forXZ( img_src, x, z )
    {
        img(x,z,0,0)=img_src(x,y,z,0);
        img(x,z,0,1)=img_src(x,y,z,1);
        img(x,z,0,2)=img_src(x,y,z,2);
    }
    return img;
}

cimg_library::CImg<uint8_t> extractZY(const cimg_library::CImg<uint8_t> &img_src, int x)
{
    cimg_library::CImg<uint8_t> img(img_src.depth(), img_src.height(),1,3);
    cimg_forYZ( img_src, y, z )
    {
        img(z,y,0,0)=img_src(x,y,z,0);
        img(z,y,0,1)=img_src(x,y,z,1);
        img(z,y,0,2)=img_src(x,y,z,2);
    }
    return img;
}
