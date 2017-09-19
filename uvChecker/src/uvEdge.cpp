#include "uvEdge.h"


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
    if (p1.index < p2.index) {
        this->edgeIndex.first = p1.index;
        this->edgeIndex.second = p2.index;
    } else {
        this->edgeIndex.first = p2.index;
        this->edgeIndex.second = p1.index;
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
