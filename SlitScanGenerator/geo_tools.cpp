#include "geo_tools.h"
#include <QDebug>

bool intersects_xrange(const line2 &l, double x0, double x1, double y, double& xintersect)
{
    if (y==l.y) {
        if (l.x>=x0 && l.x<=x1) {
            xintersect=l.x;
            qDebug()<<"intersects_xrange("<<l.x<<"/"<<l.y<<"/"<<l.angle<<", "<<x0<<", "<<x1<<", "<<y<<"): y==l.y && l.x>=x0 && l.x<=x1: true, "<<xintersect;
            return true;
        } else if (l.angle==0) {
            xintersect=x0;
            qDebug()<<"intersects_xrange("<<l.x<<"/"<<l.y<<"/"<<l.angle<<", "<<x0<<", "<<x1<<", "<<y<<"): y==l.y && l.angle==0: true, "<<xintersect;
            return true;
        }
        qDebug()<<"intersects_xrange("<<l.x<<"/"<<l.y<<"/"<<l.angle<<", "<<x0<<", "<<x1<<", "<<y<<"): y==l.y: false";
        return false;
    }
    double xi=l.x+(y-l.y)/tan(l.angle/180.0*M_PI);
    if (xi>=x0 && xi<=x1) {
        xintersect=xi;
        qDebug()<<"intersects_xrange("<<l.x<<"/"<<l.y<<"/"<<l.angle<<", "<<x0<<", "<<x1<<", "<<y<<"): true, "<<xintersect;
        return true;
    }
    qDebug()<<"intersects_xrange("<<l.x<<"/"<<l.y<<"/"<<l.angle<<", "<<x0<<", "<<x1<<", "<<y<<"): false";
    return false;

}

bool intersects_yrange(const line2 &l, double x, double y0, double y1, double& yintersect)
{
    if (x==l.x) {
        if (l.y>=y0 && l.y<=y1) {
            yintersect=l.y;
            qDebug()<<"intersects_yrange("<<l.x<<"/"<<l.y<<"/"<<l.angle<<", "<<x<<", "<<y0<<", "<<y1<<"): x==l.x && l.y>=y0 && l.y<=y1: true, "<<yintersect;
            return true;
        }
        qDebug()<<"intersects_yrange("<<l.x<<"/"<<l.y<<"/"<<l.angle<<", "<<x<<", "<<y0<<", "<<y1<<"): x==l.x: false";
        return false;
    }
    double yi=l.y+tan(l.angle/180.0*M_PI)*(x-l.x);
    if (yi>=y0 && yi<=y1) {
        yintersect=yi;
        qDebug()<<"intersects_yrange("<<l.x<<"/"<<l.y<<"/"<<l.angle<<", "<<x<<", "<<y0<<", "<<y1<<"): true, "<<yintersect;
        return true;
    }
    qDebug()<<"intersects_yrange("<<l.x<<"/"<<l.y<<"/"<<l.angle<<", "<<x<<", "<<y0<<", "<<y1<<"): false";
    return false;
}
