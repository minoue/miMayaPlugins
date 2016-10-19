#ifndef __randomPoint__randomPointOnMesh__
#define __randomPoint__randomPointOnMesh__

#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MSyntax.h>
#include <vector>


class RandomPointOnMesh : public MPxCommand
{
public:
    RandomPointOnMesh();
    virtual ~RandomPointOnMesh();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    MStatus getWeight(MFloatArray& weightArray);
    MStatus getAccumulatedArea(const std::vector<MIntArray> &v, MDoubleArray& accumulatedArea);
    MStatus getTwoVectors(const MIntArray& faceIDs, MPoint& origin, MVector& v1, MVector& v2);
    bool isUndoable() const;
    static void* creater();
    static MSyntax newSyntax();
private:
    MDagPath mDagPath;
    MFnMesh fnMesh;
    MString colorSet;
    unsigned numOfPoints;
};

#endif /* defined(__randomPoint__randomPointOnMesh__) */
