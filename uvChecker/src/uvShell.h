#ifndef __UVSHELL__
#define __UVSHELL__

#include "uvPoint.h"
#include <unordered_set>
#include <vector>
#include "uvEdge.h"
#include <set>

class UvShell {
public:
    UvShell();
    ~UvShell();

    int shellIndex;
    float uMax;
    float uMin;
    float vMax;
    float vMin;

    std::vector<UvPoint> uvPoints;
    std::vector<float> uVector;
    std::vector<float> vVector;
    std::unordered_set<int> polygonIDs;
    std::vector<int> borderUvPoints;
    std::set<UvEdge> edgeSet;

    bool operator==(const UvShell& rhs) const;
    inline bool operator!=(const UvShell& rhs) const
    {
        return !(*this == rhs);
    }

private:
};

#endif /* defined(__UVSHELL_H__) */
