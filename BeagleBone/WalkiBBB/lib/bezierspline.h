#ifndef DEF_LIB_BEZIERSPLINE_H
#define DEF_LIB_BEZIERSPLINE_H

#include <vector>
#include "vec2.h"

// Based on this post: https://stackoverflow.com/a/23258882/3501166.
class BezierSpline
{
public:
    BezierSpline();

    void computeSpline(std::vector<Vec2f> points);
    Vec2f eval(unsigned int seg, float t) const;
    Vec2f eval(float t) const;

    std::vector<Vec2f> getPoints() const;
    void getCoefs(std::vector<Vec2f> &points, std::vector<Vec2f> &p1Points,
                  std::vector<Vec2f> &p2Points) const;

private:
    std::vector<Vec2f> points;
    std::vector<Vec2f> p1Points;
    std::vector<Vec2f> p2Points;
};

#endif
