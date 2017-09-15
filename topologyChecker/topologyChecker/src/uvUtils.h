#ifndef __UVUTILS_H__
#define __UVUTILS_H__

#include <utility>

class UVPoint {
public:
    UVPoint();
    UVPoint(int uvIndex);
    UVPoint(double u, double v, int uvIndex);
    UVPoint(const UVPoint& source);
    ~UVPoint();

    std::pair<double, double> uv;
    int uvIndex;

    bool operator==(const UVPoint& rhs) const;
    inline bool operator!=(const UVPoint& rhs) const
    {
        return !(*this == rhs);
    }
    bool operator<(const UVPoint& rhs) const;

private:
};

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

class UVShell {
public:
    UVShell();
    ~UVShell();
    unsigned int shellIndex;
};

class UVEvent {
public:
    UVEvent();
    ~UVEvent();
    unsigned int eventIndex;
    UVPoint thisPonit;
    UVPoint oppositePoint;
    enum eventType {
        BEGIN,
        END,
        INTERSECTION
    };
};

#endif /* defined(__UVUTILS_H__) */
