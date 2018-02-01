#include "uvPoint.h"

UvPoint::UvPoint()
{
}

UvPoint::UvPoint(float u, float v)
{
    this->u = u;
    this->v = v;
}

UvPoint::UvPoint(float u, float v, int index)
{
    this->u = u;
    this->v = v;
    this->index = index;
}

UvPoint::UvPoint(float u, float v, int index, int shellIndex)
{
    this->u = u;
    this->v = v;
    this->index = index;
    this->shellIndex = shellIndex;
}

UvPoint::~UvPoint()
{
}

bool UvPoint::operator==(const UvPoint& rhs) const
{
    return this->index == rhs.index;
}

bool UvPoint::operator>(const UvPoint& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return false;
    else if (this->v == rhs.v)
        return this->u > rhs.u;
    else
        return this->v > rhs.v;
}

bool UvPoint::operator>=(const UvPoint& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return true;
    else if (this->v == rhs.v)
        return this->u > rhs.u;
    else
        return this->v > rhs.v;
}

bool UvPoint::operator<(const UvPoint& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return false;
    else if (this->v == rhs.v)
        return this->u < rhs.u;
    else
        return this->v < rhs.v;
}

bool UvPoint::operator<=(const UvPoint& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return true;
    else if (this->v == rhs.v)
        return this->u < rhs.u;
    else
        return this->v < rhs.v;
}
