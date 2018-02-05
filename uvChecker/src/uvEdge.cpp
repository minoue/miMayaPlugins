#include "uvEdge.h"
#include <iostream>
#include <math.h>
#include <float.h>

UvEdge::UvEdge()
{
}

UvEdge::UvEdge(UvPoint beginPt, UvPoint endPt, std::pair<int, int> index)
{
    this->begin = beginPt;
    this->end = endPt;
    this->index = index;
}

UvEdge::~UvEdge()
{
}

bool UvEdge::operator==(const UvEdge& rhs) const
{
    return this->index == rhs.index;
}

bool UvEdge::operator>(const UvEdge& rhs) const
{
    return this->index > rhs.index;
}

bool UvEdge::operator>=(const UvEdge& rhs) const
{
    return this->index >= rhs.index;
}

bool UvEdge::operator<(const UvEdge& rhs) const
{
    return this->index < rhs.index;
}

bool UvEdge::operator<=(const UvEdge& rhs) const
{
    return this->index <= rhs.index;
}

bool UvEdge::isIntersected(UvEdge& otherEdge) {
    float area1 = getTriangleArea(
        this->begin.u,
        this->begin.v,
        otherEdge.begin.u,
        otherEdge.begin.v,
        this->end.u,
        this->end.v);

    float area2 = getTriangleArea(
        this->begin.u,
        this->begin.v,
        otherEdge.end.u,
        otherEdge.end.v,
        this->end.u,
        this->end.v);

    float area3 = getTriangleArea(
        otherEdge.begin.u,
        otherEdge.begin.v,
        this->begin.u,
        this->begin.v,
        otherEdge.end.u,
        otherEdge.end.v);

    float area4 = getTriangleArea(
        otherEdge.begin.u,
        otherEdge.begin.v,
        this->end.u,
        this->end.v,
        otherEdge.end.u,
        otherEdge.end.v);

    float zero = 0.0;

    if (fabsf(zero - area1) <= FLT_EPSILON * fmaxf(1.f, fmaxf(fabsf(zero), fabsf(area1))))
    {
        area1 = 0;
    }
    if (fabsf(zero - area2) <= FLT_EPSILON * fmaxf(1.f, fmaxf(fabsf(zero), fabsf(area2))))
    {
        area2 = 0;
    }
    if (fabsf(zero - area3) <= FLT_EPSILON * fmaxf(1.f, fmaxf(fabsf(zero), fabsf(area3))))
    {
        area3 = 0;
    }
    if (fabsf(zero - area4) <= FLT_EPSILON * fmaxf(1.f, fmaxf(fabsf(zero), fabsf(area4))))
    {
        area4 = 0;
    }

    float ccw1 = area1 * area2;
    float ccw2 = area3 * area4;

    if (area1 == 0.0 && area2 == 0.0) {
        // If two edges are parallel
        Vector v1 = this->begin - otherEdge.begin;
        Vector v2 = this->end - otherEdge.begin;
        float d1 = v1.dot(v2);
        Vector v3 = this->begin - otherEdge.end;
        Vector v4 = this->end - otherEdge.end;
        float d2 = v3.dot(v4);
        if (d1 >= 0.0 and d2 >= 0.0)
            return false;
        else
            return true;
    }

    if (ccw1 < 0 && ccw2 < 0) {
        return true;
    }
    else
        return false;

}

float UvEdge::getTriangleArea(float x1,
                              float y1,
                              float x2,
                              float y2,
                              float x3,
                              float y3) {

    float U = (x1 * y2) + (x2 * y3) + (x3 * y1) - (y1 * x2) - (y2 * x3) - (y3 * x1);
    float S = U / 2.0;
    return S;
}
