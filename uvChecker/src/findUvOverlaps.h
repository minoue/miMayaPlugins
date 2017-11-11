#ifndef __FINDUVOVERLAPS_H__
#define __FINDUVOVERLAPS_H__

#include "uvPoint.h"
#include "uvShell.h"
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

    bool checkShellIntersection(UvShell& s1, UvShell& s2);
    MStatus createTaskData(int numPolygons, MString& uvSet);
    MStatus createShellTaskData(
        UvShell& shellA,
        UvShell& shellB,
        MString& uvSet,
        std::unordered_map<int,
        std::vector<int>>& uvMap);
    MStatus findShellIntersectionsST(
        UvShell& shellA,
        UvShell& shellB,
        MString* uvSetPtr,
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
    MString uvSet;
    static MDagPath mDagPath;
    static MFloatArray uArray;
    static MFloatArray vArray;
    std::vector<bool> resultBoolVector;
    MStringArray resultStrArray;
};

#endif /* defined(__FINDUVOVERLAPS_H__) */
