#ifndef GEO_TOOLS_H
#define GEO_TOOLS_H
#define _USE_MATH_DEFINES
#include <cmath>

struct line2 {
    inline line2(double x_, double y_, double angle_):
        x(x_), y(y_), angle(angle_)
    {}


    double x;
    double y;
    double angle;
};

struct stretch2 {
    inline stretch2(double x0_, double y0_, double x1_, double y1_):
        x0(x0_), y0(y0_), x1(x1_),y1(y1_)
    {}

    double dx() const {return x1-x0;}
    double dy() const {return y1-y0;}
    double angle() const { return atan(dy()/dx())/M_PI*180.0; }

    double x0;
    double y0;
    double x1;
    double y1;
};

inline bool intersects(const line2& l1, const stretch2& l2, double& xi, double&y1) {
    return false;
}

bool intersects_xrange(const line2& l, double x0, double x1, double y, double& xintersect);
bool intersects_yrange(const line2& l, double x, double y0, double y1, double& yintersect);

#endif // GEO_TOOLS_H
