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
    
    // Check edge index if they have shared UV index
    bool isConnected;
    int& this_index_A = this->index.first;
    int& this_index_B = this->index.second;
    int& other_index_A = otherEdge.index.first;
    int& other_index_B = otherEdge.index.second;
    if (this_index_A == other_index_A || this_index_A == other_index_B) {
        isConnected = true;
    }
    else if (this_index_B == other_index_A || this_index_B == other_index_B) {
        isConnected = true;
    }
    else {
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
        // If two edges are parallel on a same line
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
    
    float ccw1;
    float ccw2;
    // If two edges are connected, at least 2 area of 4 triangles should be 0,
    // therefore, ccw1 and 2 need to be 0.
    if (isConnected) {
        ccw1 = 0;
        ccw2 = 0;
    }
    else {
        ccw1 = area1 * area2;
        ccw2 = area3 * area4;
    }
    
    if (ccw1 < 0 && ccw2 < 0) {
        return true;
    }
    else
        return false;

}

float UvEdge::getTriangleArea(float& Ax, float& Ay, float& Bx, float& By, float& Cx, float& Cy)
{
    float area = ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) / (float)2.0;
    return area;
}
