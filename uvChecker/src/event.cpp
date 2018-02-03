#include "event.h"

Event::Event()
{
}

Event::Event(std::string status, UvPoint point, UvEdge edge, int index)
{
    this->status = status;
    this->point = point;
    this->edge = edge;
    this->index = index;

    this->u = point.u;
    this->v = point.v;
}

Event::~Event()
{
}

bool Event::operator==(const Event& rhs) const
{
    return this->index == rhs.index;
}

bool Event::operator>(const Event& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return false;
    else if (this->v == rhs.v)
        return this->u > rhs.u;
    else
        return this->v > rhs.v;
}

bool Event::operator>=(const Event& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return true;
    else if (this->v == rhs.v)
        return this->u > rhs.u;
    else
        return this->v > rhs.v;
}

bool Event::operator<(const Event& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return false;
    else if (this->v == rhs.v)
        return this->u < rhs.u;
    else
        return this->v < rhs.v;
}

bool Event::operator<=(const Event& rhs) const
{
    if (this->u == rhs.u && this->v == rhs.v)
        return true;
    else if (this->v == rhs.v)
        return this->u < rhs.u;
    else
        return this->v < rhs.v;
}
