#include "cimg_tools.h"
#include <cmath>
#include <QPoint>
#include <QDebug>
#include <QSet>
#include "geo_tools.h"


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


cimg_library::CImg<uint8_t> extractXZ_pitch(const cimg_library::CImg<uint8_t> &img_src, int y, double angle)
{
    if (angle==0) return extractXZ(img_src, y);
    else {
        int dy=static_cast<int>(std::ceil(double(img_src.depth())*std::sin(angle/180.0*M_PI)));
        int dymax=img_src.height()-y-1;
        //qDebug()<<"dy="<<dy;
        //qDebug()<<"dymax="<<dymax;

        int length=0;
        if (dy<=dymax) {
            length=static_cast<int>(std::ceil(double(img_src.depth())/std::cos(angle/180.0*M_PI)))-1;
            //qDebug()<<"dy<=dymax => length="<<length;
        } else {
            length=static_cast<int>(std::ceil(double(dymax)/std::sin(angle/180.0*M_PI)))-1;
            //qDebug()<<"dy>dymax => length="<<length;
        }

        const double cosa=std::cos(angle/180.0*M_PI);
        const double sina=std::sin(angle/180.0*M_PI);
        cimg_library::CImg<uint8_t> img(img_src.width(), length,1,3);
        int maxz=0;
        for (int z=0; z<length; z++) {
            int ys=y+static_cast<int>(std::round(static_cast<double>(z)*sina));
            int zs=static_cast<int>(std::round(static_cast<double>(z)*cosa));
            //qDebug()<<"z="<<z<<", ys="<<ys<<"["<<img_src.height()<<"], zs="<<zs<<"["<<img_src.depth()<<"]";
            if (ys>=0&&ys<img_src.height()&&zs>=0&&zs<img_src.depth()) {
                maxz=std::max(z,maxz);
                for (int x=0; x<img_src.width(); x++) {
                    img(x,z,0,0)=img_src(x,ys,zs,0);
                    img(x,z,0,1)=img_src(x,ys,zs,1);
                    img(x,z,0,2)=img_src(x,ys,zs,2);
                }
            }
        }
        return img.resize(img.width(),maxz+1,1,3,0);
    }
}

cimg_library::CImg<uint8_t> extractZY_pitch(const cimg_library::CImg<uint8_t> &img_src, int x, double angle)
{
    if (angle==0) return extractZY(img_src, x);
    else {
        int dx=static_cast<int>(std::ceil(double(img_src.depth())*std::sin(angle/180.0*M_PI)));
        int dxmax=img_src.width()-x-1;

        int length=0;
        if (dx<=dxmax) {
            length=static_cast<int>(std::ceil(double(img_src.depth())/std::cos(angle/180.0*M_PI)))-1;
        } else {
            length=static_cast<int>(std::ceil(double(dxmax)/std::sin(angle/180.0*M_PI)))-1;
        }

        const double cosa=std::cos(angle/180.0*M_PI);
        const double sina=std::sin(angle/180.0*M_PI);
        cimg_library::CImg<uint8_t> img(length,img_src.height(), 1,3);
        int maxz=0;
        for (int z=0; z<length; z++) {
            int xs=x+static_cast<int>(std::round(static_cast<double>(z)*sina));
            int zs=static_cast<int>(std::round(static_cast<double>(z)*cosa));
            //qDebug()<<"xs="<<xs<<"["<<img_src.width()<<"], z="<<z<<"["<<length<<"], zs="<<zs<<"["<<img_src.depth()<<"]";
            if (xs>=0&&xs<img_src.width()&&zs>=0&&zs<img_src.depth()) {
                maxz=std::max(z,maxz);
                for (int y=0; y<img_src.height(); y++) {
                    img(z,y,0,0)=img_src(xs,y,zs,0);
                    img(z,y,0,1)=img_src(xs,y,zs,1);
                    img(z,y,0,2)=img_src(xs,y,zs,2);
                }
            }
        }
        return img.resize(maxz+1,img.height(),1,3,0);
    }


}



static inline uint qHash(const QPoint &p, uint /*seed*/) {
    return static_cast<uint>(abs(p.x()+p.y()));
}

static inline uint qHash(const QPointF &p, uint /*seed*/) {
    return static_cast<uint>(abs(p.x()+p.y()));
}

#define sqr(x) ((x)*(x))

cimg_library::CImg<uint8_t> extractXZ_roll(const cimg_library::CImg<uint8_t> &img_src, int x, int y, double angle)
{
    if (angle==0) return extractXZ(img_src, y);
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
        qDebug()<<"==================================================================";
        qDebug()<<"img: "<<img_src.width()<<"x"<<img_src.height()<<"x"<<img_src.depth()<<"x"<<img_src.spectrum();
        /*for (auto p: points) {
            qDebug()<<"p="<<p;
        }*/

        if (points.size()>=2) {
            QPointF p1=*(points.begin());
            QPointF p2=*(points.rbegin());
            //if (QPointF::dotProduct(p1, QPointF(0,0))>QPointF::dotProduct(p2, QPointF(0,0))) qSwap(p1,p2);
            if (p1.x()>p2.x()) qSwap(p1,p2);

            int length=static_cast<int>(std::round(std::sqrt(sqr(p1.x()-p2.x())+sqr(p1.y()-p2.y()))));
            qDebug()<<"p1="<<p1<<", p2="<<p2<<", l="<<length;

            //const double cosa=std::cos(angle/180.0*M_PI);
            //const double sina=std::sin(angle/180.0*M_PI);
            cimg_library::CImg<uint8_t> img(length, img_src.depth(),1,3);
            //int minx=length;
            //int maxx=-1;
            for (int z=0; z<img_src.depth(); z++) {
                for (int l=0; l<length; l++) {
                    float xs=static_cast<double>(p1.x())+static_cast<double>(l)*(p2.x()-p1.x())/static_cast<double>(length);
                    float ys=static_cast<double>(p1.y())+static_cast<double>(l)*(p2.y()-p1.y())/static_cast<double>(length);
                    //if (z==0) qDebug()<<"xs="<<xs<<", ys="<<ys;
                    if (xs>=0 && xs<img_src.width() && ys>0 && ys<img_src.height()) {
                        //minx=std::min(minx,l);
                        //maxx=std::max(maxx,l);
                        img(l,z,0,0)=img_src.linear_atXY(xs,ys,z,0);
                        img(l,z,0,1)=img_src.linear_atXY(xs,ys,z,1);
                        img(l,z,0,2)=img_src.linear_atXY(xs,ys,z,2);
                    }
                }
            }
            qDebug()<<"img_out: "<<img.width()<<"x"<<img.height()<<"x"<<img.depth()<<"x"<<img.spectrum();
            return img;
        } else {
            return extractXZ(img_src, y);
        }
    }
}

cimg_library::CImg<uint8_t> extractZY_roll(const cimg_library::CImg<uint8_t> &img_src, int x, int y, double angle)
{
    if (angle==0) return extractZY(img_src, x);
    else return extractXZ_roll(img_src, x, y, angle-90.0);
}
