#include "cimg_tools.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <QPoint>
#include <QDebug>
#include <QSet>
#include "geo_tools.h"

static inline uint qHash(const QPoint &p, uint /*seed*/) {
    return static_cast<uint>(abs(p.x()+p.y()));
}

static inline uint qHash(const QPointF &p, uint /*seed*/) {
    return static_cast<uint>(abs(p.x()+p.y()));
}

#define sqr(x) ((x)*(x))

template<class T>
static inline bool approx0(T v, double limit=0.0001) {
    return std::fabs(double(v))<limit;
}

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

cimg_library::CImg<uint8_t> extractXZ_atz(int z, const cimg_library::CImg<uint8_t> &img_src, int y)
{
    cimg_library::CImg<uint8_t> img(img_src.width(), 1,1,3);
    cimg_forX( img_src, x )
    {
        img(x,0,0,0)=img_src(x,y,0,0);
        img(x,0,0,1)=img_src(x,y,0,1);
        img(x,0,0,2)=img_src(x,y,0,2);
    }
    return img;
}

cimg_library::CImg<uint8_t> extractZY_atz(int z, const cimg_library::CImg<uint8_t> &img_src, int x)
{
    cimg_library::CImg<uint8_t> img(img_src.height(),1, 1,3);
    cimg_forY( img_src, y)
    {
        img(y,0,0,0)=img_src(x,y,0,0);
        img(y,0,0,1)=img_src(x,y,0,1);
        img(y,0,0,2)=img_src(x,y,0,2);
    }
    return img;
}



cimg_library::CImg<uint8_t> extractXZ_atz_pitch(int z, int depth, const cimg_library::CImg<uint8_t> &img_src, int y, double angle, const interpolatingAtXYFunctor &atFunc, int& zout, int* lenout)
{
    if (approx0(angle)) {
        if (lenout) *lenout=depth;
        return extractXZ_atz(z, img_src, y);
    } else {
        //qDebug()<<"y="<<y<<", img="<<img_src.width()<<"x"<<img_src.height()<<"x"<<depth<<", angle="<<angle<<", video_xyFactor="<<video_xyFactor<<", video_everyNthFrame="<<video_everyNthFrame;
        int dy=static_cast<int>(std::ceil(double(depth)*std::tan(angle/180.0*M_PI)));
        int dymax=img_src.height()-y-1;
        //qDebug()<<"dy="<<dy;
        //qDebug()<<"dymax="<<dymax;

        int length=0;
        double ystart=y;
        double zstart=0;
        double yend=y;
        double zend=0;
        if (dy<=dymax) {
            yend=y+dy;
            zend=depth;
            length=static_cast<int>(std::ceil(sqrt(sqr(zend-zstart)+sqr(yend-ystart))));
            //qDebug()<<"dy<=dymax => length="<<length;
        } else {
            yend=y+dymax;
            zend=double(dymax)/std::tan(angle/180.0*M_PI);
            length=static_cast<int>(std::ceil(sqrt(sqr(zend-zstart)+sqr(yend-ystart))));
            //qDebug()<<"dy>dymax => length="<<length;
        }
        //qDebug()<<"ystart="<<ystart<<", yend="<<yend<<", zstart="<<zstart<<", zend="<<zend<<", length="<<length;



        cimg_library::CImg<uint8_t> img(img_src.width(), 1,1,3);
        int zs=z;
        int zimg=0;
        int oldzout=zout;
        while ((zs=static_cast<int>(std::round(zstart+static_cast<double>(zout)/double(length)*(zend-zstart))))==z) {
            const float ys=ystart+static_cast<double>(zout)/double(length)*(yend-ystart);
            if (ys>=0&&ys<img_src.height()) {
                zimg++;
            }
            zout++;
        }
        if (zimg>=img.height()) {
            // grow if necessary
            img.resize(img.width(), zimg, 1, 3);
        }
        zimg=0;
        zout=oldzout;
        while ((zs=static_cast<int>(std::round(zstart+static_cast<double>(zout)/double(length)*(zend-zstart))))==z) {
            const float ys=ystart+static_cast<double>(zout)/double(length)*(yend-ystart);
            //qDebug()<<"z="<<z<<"<<"", zout="<<zout<<", ys="<<ys<<"["<<img_src.height()<<"], zs="<<zs<<"["<<depth<<"]";
            if (ys>=0&&ys<img_src.height()) {//&&zs>=0&&zs<depth) {

                for (int x=0; x<img_src.width(); x++) {
                    img(x,zimg,0,0)=atFunc(img_src,x,ys,0,0);
                    img(x,zimg,0,1)=atFunc(img_src,x,ys,0,1);
                    img(x,zimg,0,2)=atFunc(img_src,x,ys,0,2);
                }
                zimg++;
            }
            zout++;
        }
        if (zimg<=0) return cimg_library::CImg<uint8_t>();
        return img;
    }
}

cimg_library::CImg<uint8_t> extractZY_atz_pitch(int z, int depth, const cimg_library::CImg<uint8_t> &img_src, int x, double angle, const interpolatingAtXYFunctor &atFunc, int& zout, int* lenout)
{
    if (approx0(angle)) {
        if (lenout) *lenout=depth;
        return extractZY_atz(z, img_src, x);
    } else {
        int dx=static_cast<int>(std::ceil(double(depth)*std::tan(angle/180.0*M_PI)));
        int dxmax=img_src.width()-x-1;
        //qDebug()<<"dy="<<dy;
        //qDebug()<<"dymax="<<dymax;

        int length=0;
        double xstart=x;
        double zstart=0;
        double xend=x;
        double zend=0;
        if (dx<=dxmax) {
            xend=x+dx;
            zend=depth;
            length=static_cast<int>(std::ceil(sqrt(sqr(zend-zstart)+sqr(xend-xstart))));
            //qDebug()<<"dy<=dymax => length="<<length;
        } else {
            xend=x+dxmax;
            zend=double(dxmax)/std::tan(angle/180.0*M_PI);
            length=static_cast<int>(std::ceil(sqrt(sqr(zend-zstart)+sqr(xend-xstart))));
            //qDebug()<<"dy>dymax => length="<<length;
        }


        cimg_library::CImg<uint8_t> img(img_src.height(),1, 1,3);
        int zs=z;
        int zimg=0;
        int oldzout=zout;
        while ((zs=static_cast<int>(std::round(zstart+static_cast<double>(zout)/double(length)*(zend-zstart))))==z) {
            const float xs=xstart+static_cast<double>(zout)/double(length)*(xend-xstart);
            if (xs>=0&&xs<img_src.width()) {
                zimg++;
            }
            zout++;
        }
        if (zimg>=img.height()) {
            // grow if necessary
            img.resize(img.width(), zimg, 1, 3);
        }
        zimg=0;
        zout=oldzout;
        while ((zs=static_cast<int>(std::round(zstart+static_cast<double>(zout)/double(length)*(zend-zstart))))==z) {
            const float xs=xstart+static_cast<double>(zout)/double(length)*(xend-xstart);

            //qDebug()<<"xs="<<xs<<"["<<img_src.width()<<"], z="<<z<<"["<<length<<"], zs="<<zs<<"["<<depth<<"]";
            if (xs>=0&&xs<img_src.width()) {//&&zs>=0&&zs<depth) {
                for (int y=0; y<img_src.height(); y++) {
                    img(y,zimg,0,0)=atFunc(img_src,xs,y,0,0);
                    img(y,zimg,0,1)=atFunc(img_src,xs,y,0,1);
                    img(y,zimg,0,2)=atFunc(img_src,xs,y,0,2);
                }
                zimg++;
            }
            zout++;
        }
        if (zimg<=0) return cimg_library::CImg<uint8_t>();
        return img;
    }


}





cimg_library::CImg<uint8_t> extractXZ_atz_roll(int z, int depth, const cimg_library::CImg<uint8_t> &img_src, int x, int y, double angle, const interpolatingAtXYFunctor &atFunc)
{
    if (approx0(angle)) return extractXZ_atz(z, img_src, y);
    else {
        double x0=0,y0=0;
        line2 l(x,y,angle);
        QSet<QPointF> points;
        x0=0;
        y0=0;
        if (intersects_xrange(l, 0, img_src.width()-1, y0, x0)) points.insert(QPointF(x0, y0));
        x0=0;
        y0=img_src.height()-1;
        if (intersects_xrange(l, 0, img_src.width()-1, y0, x0)) points.insert(QPointF(x0, y0));
        x0=0;
        y0=0;
        if (intersects_yrange(l, x0, 0, img_src.height()-1, y0)) points.insert(QPointF(x0, y0));
        x0=img_src.width()-1;
        y0=0;
        if (intersects_yrange(l, x0, 0, img_src.height()-1, y0)) points.insert(QPointF(x0, y0));
        //qDebug()<<"==================================================================";
        //qDebug()<<"img: "<<img_src.width()<<"x"<<img_src.height()<<"x"<<depth<<"x"<<img_src.spectrum();
        /*for (auto p: points) {
            //qDebug()<<"p="<<p;
        }*/

        if (points.size()>=2) {
            const auto ps=points.values();
            QPointF p1=ps.first();
            QPointF p2=ps.last();
            //if (QPointF::dotProduct(p1, QPointF(0,0))>QPointF::dotProduct(p2, QPointF(0,0))) qSwap(p1,p2);
            if (p1.x()>p2.x()) qSwap(p1,p2);

            int length=static_cast<int>(std::round(std::sqrt(sqr(p1.x()-p2.x())+sqr(p1.y()-p2.y()))));
            //qDebug()<<"p1="<<p1<<", p2="<<p2<<", l="<<length;

            //const double cosa=std::cos(angle/180.0*M_PI);
            //const double sina=std::sin(angle/180.0*M_PI);
            cimg_library::CImg<uint8_t> img(length, 1,1,3);
            //int minx=length;
            //int maxx=-1;
            for (int l=0; l<length; l++) {
                float xs=static_cast<double>(p1.x())+static_cast<double>(l)*(p2.x()-p1.x())/static_cast<double>(length);
                float ys=static_cast<double>(p1.y())+static_cast<double>(l)*(p2.y()-p1.y())/static_cast<double>(length);
                //if (z==0) //qDebug()<<"xs="<<xs<<", ys="<<ys;
                if (xs>=0 && xs<img_src.width() && ys>0 && ys<img_src.height()) {
                    img(l,0,0,0)=atFunc(img_src,xs,ys,0,0);
                    img(l,0,0,1)=atFunc(img_src,xs,ys,0,1);
                    img(l,0,0,2)=atFunc(img_src,xs,ys,0,2);
                }
            }
            //qDebug()<<"img_out: "<<img.width()<<"x"<<img.height()<<"x"<<img.depth()<<"x"<<img.spectrum();
            return img;
        } else {
            return extractXZ_atz(z,img_src, y);
        }
    }
}

cimg_library::CImg<uint8_t> extractZY_atz_roll(int z, int depth, const cimg_library::CImg<uint8_t> &img_src, int x, int y, double angle, const interpolatingAtXYFunctor &atFunc)
{
    if (approx0(angle)) return extractZY_atz(z, img_src, x);
    else return extractXZ_atz_roll(z, depth, img_src, x, y, angle-90.0, atFunc);
}
