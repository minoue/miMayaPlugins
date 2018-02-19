#ifndef __UVEVENT__
#define __UVEVENT__

#include "uvEdge.h"
#include "uvPoint.h"
#include <string>

class Event {
public:
    Event();
    Event(std::string status, UvPoint point, UvEdge& edge, int index);
    Event(std::string status, float u, float v, UvEdge& edge, UvEdge& otherEdge);
    ~Event();

    std::string status;
    UvPoint point;
    UvEdge edge;
    UvEdge otherEdge;
    int index;

    float u;
    float v;

    bool operator==(const Event& rhs) const;
    inline bool operator!=(const Event& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator>(const Event& rhs) const;
    bool operator>=(const Event& rhs) const;
    bool operator<(const Event& rhs) const;
    bool operator<=(const Event& rhs) const;

private:
};

#endif /* defined(__UVEVENT_H__) */
