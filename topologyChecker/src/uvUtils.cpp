#include "uvUtils.h"

// UV point imprementation
UVPoint::UVPoint()
{
}

UVPoint::UVPoint(int uvIndex)
{
    this->uvIndex = uvIndex;
}

UVPoint::UVPoint(double u, double v, int uvIndex)
{
    this->uvIndex = uvIndex;
    this->uv.first = u;
    this->uv.second = v;
}

UVPoint::UVPoint(const UVPoint& source)
{
    uv = source.uv;
    uvIndex = source.uvIndex;
}

UVPoint::~UVPoint()
{
}

bool UVPoint::operator==(const UVPoint& rhs) const
{
    return this->uvIndex == rhs.uvIndex;
}

bool UVPoint::operator<(const UVPoint& rhs) const
{
    return this->uv < rhs.uv;
}

// UV EDGE imprementation
UVEdge::UVEdge()
{
}

UVEdge::UVEdge(UVPoint p1, UVPoint p2)
{
    // Set begin point and end point of edge
    if (p1 < p2) {
        this->begin = p1;
        this->end = p2;
    } else {
        this->begin = p2;
        this->end = p1;
    }

    // Set edge index(A pair of indecis of each UVpoints)
    if (p1.uvIndex < p2.uvIndex) {
        this->edgeIndex.first = p1.uvIndex;
        this->edgeIndex.second = p2.uvIndex;
    } else {
        this->edgeIndex.first = p2.uvIndex;
        this->edgeIndex.second = p1.uvIndex;
    }
}

UVEdge::UVEdge(const UVEdge& edgeSource)
{
    begin = edgeSource.begin;
    end = edgeSource.end;
    edgeIndex = edgeSource.edgeIndex;
}

UVEdge::~UVEdge()
{
}

bool UVEdge::operator==(const UVEdge& rhs) const
{
    return this->edgeIndex == rhs.edgeIndex;
}

bool UVEdge::operator<(const UVEdge& refEdge) const
{
    // return this->edgeIndex < refEdge.edgeIndex;
    if (this->begin != refEdge.begin) {
        return this->begin < refEdge.begin;
    } else {
        return this->end < refEdge.end;
    }
}

bool UVEdge::hasIntersection(UVPoint& intersection)
{
    return false;
}

// UVShell imprementation
UVShell::UVShell()
{
}

UVShell::~UVShell()
{
}

// UVEvent
UVEvent::UVEvent()
{
}

UVEvent::~UVEvent()
{
}
