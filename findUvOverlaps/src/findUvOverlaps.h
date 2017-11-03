#ifndef __FINDUVOVERLAPS_H__
#define __FINDUVOVERLAPS_H__

#include <maya/MDagPath.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>
#include <maya/MIntArray.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MSyntax.h>
#include <maya/MThreadPool.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

typedef struct _uvPointData {
    float u;
    float v;
    int index;
    int shellIndex;
} UVPoint;

typedef struct _uvShellDataTag {
    std::vector<UVPoint> uvPoints;
    std::vector<float> uVector;
    std::vector<float> vVector;
    std::unordered_set<int> polygonIDs;
    std::vector<int> borderUvPoints;
    int shellIndex;
    float uMax;
    float uMin;
    float vMax;
    float vMin;
} UVShell;

class FindUvOverlaps : public MPxCommand {
public:
    FindUvOverlaps();
    virtual ~FindUvOverlaps();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creater();
    static MSyntax newSyntax();

    bool checkShellIntersection(UVShell& s1, UVShell& s2);
    MStatus createTaskData(int numPolygons);
    MStatus createShellTaskData(
        UVShell& shellA,
        UVShell& shellB,
        std::unordered_map<int,
        std::vector<int>>& uvMap);
    MStatus findShellIntersectionsST(UVShell& shellA,
        UVShell& shellB,
        std::unordered_map<int, std::vector<int>>& uvMap,
        std::vector<bool>& resultBoolVector);

    static bool checkCrossingNumber(float& u, float& v, std::vector<int>& uvIds);
    static void createThreadData(void* data, MThreadRootTask* root);
    static void createShellThreadData(void* data, MThreadRootTask* root);
    static MThreadRetVal findShellIntersectionsMT(void* data);
    static MThreadRetVal findInnerIntersectionsMT(void* data);
    static float getTriangleArea(float& Ax, float& Ay, float& Bx, float& By, float& Cx, float& Cy);

private:
    MIntArray innerIntersectionsResult;
    MIntArray shellIntersectionsResult;
    MFnMesh fnMesh;
    bool verbose;
    bool isMultiThreaded;
    static MDagPath mDagPath;
    static MFloatArray uArray;
    static MFloatArray vArray;
    std::vector<bool> resultBoolVector;
    MStringArray resultStrArray;
};

#endif /* defined(__FINDUVOVERLAPS_H__) */
