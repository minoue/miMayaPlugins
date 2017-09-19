#include "uvPoint.h"

UVPoint::UVPoint()
{
}

UVPoint::UVPoint(int index)
{
    this->uv.first = u;
    this->uv.second = v;
    this->index = index;
}

UVPoint::UVPoint(float u, float v)
{
    this->u = u;
    this->v = v;
    this->uv.first = u;
    this->uv.second = v;
}

UVPoint::UVPoint(double u, double v, int index)
{
    this->index = index;
    this->u = u;
    this->v = v;
    this->uv.first = u;
    this->uv.second = v;
}

UVPoint::UVPoint(const UVPoint& source)
{
    uv = source.uv;
    u = source.u;
    v = source.v;
    index = source.index;
}

UVPoint::~UVPoint()
{
}

bool UVPoint::operator==(const UVPoint& rhs) const
{
    return this->index == rhs.index;
}

bool UVPoint::operator<(const UVPoint& rhs) const
{
    return this->uv < rhs.uv;
}
