#include "uvEdge.h"
#include <float.h>
#include <iostream>
#include <math.h>
#include <maya/MGlobal.h>

UvEdge::UvEdge()
{
}

UvEdge::UvEdge(UvPoint beginPt, UvPoint endPt, std::pair<int, int> index)
{
    this->begin = beginPt;
    this->end = endPt;
    this->index = index;
    this->beginIndex = beginPt.index;
    this->endIndex = endPt.index;
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
    if (this->crossingPointX == rhs.crossingPointX) {
        return this->end.u > rhs.end.u;

    } else {
        return this->crossingPointX > rhs.crossingPointX;
    }
}

bool UvEdge::operator>=(const UvEdge& rhs) const
{
    if (this->crossingPointX == rhs.crossingPointX) {
        return this->end.u >= rhs.end.u;

    } else {
        return this->crossingPointX >= rhs.crossingPointX;
    }
}

bool UvEdge::operator<(const UvEdge& rhs) const
{
    if (this->crossingPointX == rhs.crossingPointX) {
        return this->end.u < rhs.end.u;

    } else {
        return this->crossingPointX < rhs.crossingPointX;
    }
}

bool UvEdge::operator<=(const UvEdge& rhs) const
{
    if (this->crossingPointX == rhs.crossingPointX) {
        return this->end.u <= rhs.end.u;

    } else {
        return this->crossingPointX <= rhs.crossingPointX;
    }
}

bool UvEdge::isIntersected(UvEdge& otherEdge, bool isParallel, float& u, float& v)
{

    // Check edge index if they have shared UV index
    bool isConnected;
    int& this_index_A = this->index.first;
    int& this_index_B = this->index.second;
    int& other_index_A = otherEdge.index.first;
    int& other_index_B = otherEdge.index.second;
    if (this_index_A == other_index_A || this_index_A == other_index_B) {
        isConnected = true;
    } else if (this_index_B == other_index_A || this_index_B == other_index_B) {
        isConnected = true;
    } else {
        isConnected = false;
    }

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

    if (area1 == 0.0 && area2 == 0.0) {
        isParallel = true;
        float u_min;
        float u_max;
        float v_min;
        float v_max;

        // Get u_min and u_max
        if (otherEdge.begin.u > otherEdge.end.u) {
            u_max = otherEdge.begin.u;
            u_min = otherEdge.end.u;
        } else {
            u_max = otherEdge.end.u;
            u_min = otherEdge.begin.u;
        }

        // Get v_min and v_max
        if (otherEdge.begin.v < otherEdge.end.v) {
            v_min = otherEdge.begin.v;
            v_max = otherEdge.end.v;
        } else {
            v_min = otherEdge.end.v;
            v_max = otherEdge.begin.v;
        }

        // If two lines are in vertical line
        if (this->begin.u == this->end.u) {
            if (v_min < this->begin.v && this->begin.v < v_max) {
                return true;
            } else if (v_min < this->end.v && this->end.v < v_max) {
                return true;
            } else {
                return false;
            }
        }

        if (u_min < this->begin.u && this->begin.u < u_max) {
            // parallel edges overlaps
            return true;
        } else if (u_min < this->end.u && this->end.u < u_max) {
            // parallel edges overlaps
            return true;
        } else {
            return false;
        }

        // If two edges are parallel on a same line
        // Vector v1 = this->begin - otherEdge.begin;
        // Vector v2 = this->end - otherEdge.begin;
        // float d1 = v1.dot(v2);
        // Vector v3 = this->begin - otherEdge.end;
        // Vector v4 = this->end - otherEdge.end;
        // float d2 = v3.dot(v4);
        // if (d1 > 0.0 && d2 > 0.0)
        //     return true;
        // else
        //     return false;
    }

    float ccw1;
    float ccw2;
    // If two edges are connected, at least 2 area of 4 triangles should be 0,
    // therefore, ccw1 and 2 need to be 0.
    if (isConnected) {
        ccw1 = 0;
        ccw2 = 0;
    } else {
        ccw1 = area1 * area2;
        ccw2 = area3 * area4;
    }

    if (ccw1 < 0 && ccw2 < 0) {
        // if two non-connected edges intersect, get intersection point values

        // for this edge
        float slopeA = (this->end.v - this->begin.v) / (this->end.u - this->begin.u);
        float y_interceptA = this->begin.v - (slopeA * this->begin.u);

        // for otherEdge
        float slopeB = (otherEdge.end.v - otherEdge.begin.v) / (otherEdge.end.u - otherEdge.begin.u);
        float y_interceptB = otherEdge.begin.v - (slopeB * otherEdge.begin.u);

        // [ -slopeA 1 ] [x] = [ y_interceptA ]
        // [ -slopeB 1 ] [y]   [ y_interceptB ]
        // Get inverse matrix

        // Negate slope values
        slopeA = -1.0F * slopeA;
        slopeB = -1.0F * slopeB;
        float adbc = slopeA - slopeB;
        float a = 1.0F * (1.0F / adbc);
        float b = -slopeB * (1.0F / adbc);
        float c = -1.0F * (1.0F / adbc);
        float d = slopeA * (1.0F / adbc);

        // [u] = [ a c ] [y_interceptA]
        // [v]   [ b d ] [y_interceptB]
        u = (a * y_interceptA) + (c * y_interceptB);
        v = (b * y_interceptA) + (d * y_interceptB);

        return true;
    } else
        return false;
}

float UvEdge::getTriangleArea(float& Ax, float& Ay, float& Bx, float& By, float& Cx, float& Cy)
{
    float area = ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) / (float)2.0;
    return area;
}

void UvEdge::setCrossingPointX(float Y)
{
    float& x1 = this->begin.u;
    float& y1 = this->begin.v;
    float& x2 = this->end.u;
    float& y2 = this->end.v;

    if (y2 == y1) {
        this->crossingPointX = this->begin.u;
    } else {
        float X = ((Y - y1) * (x2 - x1)) / (y2 - y1) + x1;
        this->crossingPointX = X;
    }
}
