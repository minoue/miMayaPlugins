#ifndef __UVEDGE__
#define __UVEDGE__

#include "uvPoint.h"
#include <utility>

class UvEdge {
public:
    UvEdge();
    UvEdge(UvPoint beginPt, UvPoint endPt, std::pair<int, int> index);
    ~UvEdge();

    UvPoint begin;
    UvPoint end;
    std::pair<int, int> index;
    int beginIndex;
    int endIndex;

    bool operator==(const UvEdge& rhs) const;
    inline bool operator!=(const UvEdge& rhs) const
    {
        return !(*this == rhs);
    }
    bool operator>(const UvEdge& rhs) const;
    bool operator>=(const UvEdge& rhs) const;
    bool operator<(const UvEdge& rhs) const;
    bool operator<=(const UvEdge& rhs) const;

    void setCrossingPointX(float Y);

    bool isIntersected(UvEdge& otherEdge, bool isParallel, float& u, float& v);
    float getTriangleArea(float& x1,
        float& y1,
        float& x2,
        float& y2,
        float& x3,
        float& y3);

private:
    float crossingPointX;
};

#endif /* defined(__UVEDGE_H__) */
