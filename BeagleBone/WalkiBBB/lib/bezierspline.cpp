#include "bezierspline.h"

#include <stdexcept>
#include <cmath>

#include <iostream> // TEMP

using namespace std;

BezierSpline::BezierSpline()
{

}

void BezierSpline::computeSpline(vector<Vec2f> points)
{
    this->points = points;
    const vector<Vec2f>& p = points;

    int N = (int)p.size()-1;

    if(N <= 0)
        throw std::runtime_error("BezierSpline::computeSpline(): not enough points.");

    p1Points.resize(N);
    p2Points.resize(N);

    if(N == 1)
    {
        // Only two points => straight line.
        p1Points[0] = (2.0f/3.0f*p[0] + 1.0f/3.0f*p[1]);
        p2Points[0] = 2.0f*p1Points[0] - p[0];
    }

    //
    vector<Vec2f> a(N);
    vector<Vec2f> b(N);
    vector<Vec2f> c(N);
    vector<Vec2f> r(N);

    // First segment.
    a[0].x = 0.0f;
    b[0].x = 2.0f;
    c[0].x = 1.0f;
    r[0].x = p[0].x + 2.0f * p[1].x;

    a[0].y = 0.0f;
    b[0].y = 2.0f;
    c[0].y = 1.0f;
    r[0].y = p[0].y + 2.0f * p[1].y;

    // Internal segments.
    for (int i = 1; i < N - 1; i++)
    {
        a[i].x = 1.0f;
        b[i].x = 4.0f;
        c[i].x = 1.0f;
        r[i].x = 4.0f * p[i].x + 2.0f * p[i+1].x;

        a[i].y = 1.0f;
        b[i].y = 4.0f;
        c[i].y = 1.0f;
        r[i].y = 4.0f * p[i].y + 2.0f * p[i+1].y;
    }

    // Last segment.
    a[N-1].x = 2.0f;
    b[N-1].x = 7.0f;
    c[N-1].x = 0.0f;
    r[N-1].x = 8.0f * p[N-1].x + p[N].x;

    a[N-1].y = 2.0f;
    b[N-1].y = 7.0f;
    c[N-1].y = 0.0f;
    r[N-1].y = 8.0f * p[N-1].y + p[N].y;

    // Solves Ax=b with the Thomas algorithm
    // (from https://en.wikipedia.org/wiki/Tridiagonal_matrix_algorithm).
    for(int i = 1; i < N; i++)
    {
        float m;

        m = a[i].x / b[i-1].x;
        b[i].x = b[i].x - m * c[i - 1].x;
        r[i].x = r[i].x - m * r[i-1].x;

        m = a[i].y / b[i-1].y;
        b[i].y = b[i].y - m * c[i - 1].y;
        r[i].y = r[i].y - m * r[i-1].y;
    }

    p1Points[N-1].x = r[N-1].x / b[N-1].x;
    p1Points[N-1].y = r[N-1].y / b[N-1].y;

    for(int i = N - 2; i >= 0; --i)
    {
        p1Points[i].x = (r[i].x - c[i].x * p1Points[i+1].x) / b[i].x;
        p1Points[i].y = (r[i].y - c[i].y * p1Points[i+1].y) / b[i].y;
    }

    // Compute P2.
    for(int i=0; i<N-1; i++)
    {
        p2Points[i].x = 2.0f * p[i+1].x - p1Points[i+1].x;
        p2Points[i].y = 2.0f * p[i+1].y - p1Points[i+1].y;
    }

    p2Points[N-1].x = 0.5f * (p[N].x+p1Points[N-1].x);
    p2Points[N-1].y = 0.5f * (p[N].y+p1Points[N-1].y);
}

Vec2f BezierSpline::eval(unsigned int seg, float t) const
{
    if(points.empty())
        throw std::runtime_error("BezierSpline::eval(): empty spline.");

    if(seg >= points.size() || seg >= p1Points.size() || seg >= p2Points.size())
        throw std::runtime_error("BezierSpline::eval(): seg too large.");

    float omt = 1.0f - t;

    Vec2f p0 = points[seg];
    Vec2f p1 = p1Points[seg];
    Vec2f p2 = p2Points[seg];
    Vec2f p3 = points[seg+1];

    float xVal = omt*omt*omt*p0.x + 3*omt*omt*t*p1.x + 3*omt*t*t*p2.x + t*t*t*p3.x;
    float yVal = omt*omt*omt*p0.y + 3*omt*omt*t*p1.y + 3*omt*t*t*p2.y + t*t*t*p3.y;

    return Vec2f(xVal, yVal);
}

Vec2f BezierSpline::eval(float t) const
{
    // Determine the relevant segment of the Bezier spline.
    const int nSegments = points.size() - 1;
    int segmentIndex = (int)(t*(float)nSegments);
    float tSegment = (t*(float)nSegments) - floor(t*(float)nSegments);

    // Clamp the value if it is out of range.
    if(tSegment < 0)
    {
        segmentIndex = 0;
        tSegment = 0.0f;
    }
    else if(segmentIndex >= nSegments)
    {
        segmentIndex = nSegments - 1;
        tSegment = 1.0f;
    }

    // Evaluate the Bezier spline.
    return eval(segmentIndex, tSegment);
}

vector<Vec2f>BezierSpline::getPoints() const
{
    return points;
}

void BezierSpline::getCoefs(vector<Vec2f> &points, vector<Vec2f> &p1Points,
                            vector<Vec2f> &p2Points) const
{
    points = this->points;
    p1Points = this->p1Points;
    p2Points = this->p2Points;
}
