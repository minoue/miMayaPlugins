#ifndef __FINDUVOVERLAPS_H__
#define __FINDUVOVERLAPS_H__

#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MIntArray.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>
#include <maya/MThreadPool.h>
#include <set>
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
    std::set<int> polygonIDs;
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

    MIntArray innerIntersectionsResult;
    MIntArray shellIntersectionsResult;

    MStatus createTaskData(int numPolygons, MString fullPath);
    bool checkShellIntersection(UVShell& s1, UVShell& s2);
    MStatus findShellIntersections(UVShell& shellA, UVShell& shellB);
    static void createThreadData(void* data, MThreadRootTask* root);
    static MThreadRetVal findInnerIntersectionsMT(void* data);

private:
    static MDagPath mDagPath;
    MFnMesh fnMesh;
    bool verbose;
};

#endif /* defined(__FINDUVOVERLAPS_H__) */
