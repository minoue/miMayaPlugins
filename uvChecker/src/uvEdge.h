#ifndef __UVEDGE
#define __UVEDGE

#include "uvPoint.h"


class UVEdge {
public:
    UVEdge();
    UVEdge(UVPoint p1, UVPoint p2);
    UVEdge(const UVEdge& edgeSource);
    ~UVEdge();

    UVPoint begin;
    UVPoint end;
    std::pair<int, int> edgeIndex;

    bool operator==(const UVEdge& rhs) const;
    inline bool operator!=(const UVEdge& rhs) const
    {
        return !(*this == rhs);
    }
    bool operator<(const UVEdge& rhs) const;

    bool hasIntersection(UVPoint& intersection);

private:
};

#endif /* defined(__UVEDGE_H__) */
