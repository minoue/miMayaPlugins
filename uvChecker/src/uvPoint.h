#ifndef __UVPOINT_H__
#define __UVPOINT_H__

#include <utility>

class UVPoint {
public:
    UVPoint();
    UVPoint(int index);
    UVPoint(float u, float v);
    UVPoint(double u, double v, int index);
    UVPoint(const UVPoint& source);
    ~UVPoint();

    std::pair<double, double> uv;
    float u;
    float v;
    int index;

    bool operator==(const UVPoint& rhs) const;
    inline bool operator!=(const UVPoint& rhs) const
    {
        return !(*this == rhs);
    }
    bool operator<(const UVPoint& rhs) const;

private:
};

#endif /* defined(__UVPOINT_H__) */
