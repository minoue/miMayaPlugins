#include "uvChecker.h"
#include "uvPoint.h"
#include <map>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>

UvChecker::UvChecker()
{
}

UvChecker::~UvChecker()
{
}

MStatus UvChecker::doIt(const MArgList& args)
{
    MStatus status;

    if (args.length() != 1) {
        MGlobal::displayError("Need one arg");
        return MStatus::kFailure;
    }

    // arg
    MString argument = args.asString(0, &status);
    if (status != MS::kSuccess) {
        return MStatus::kFailure;
    }
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MSelectionList sel;
    sel.add(argument);
    sel.getDagPath(0, mDagPath);

    return redoIt();
}

MStatus UvChecker::redoIt()
{
    MItMeshPolygon itPoly(mDagPath);
    MStringArray resultArray;

    int numTriangles;
    int numVertices;
    MIntArray vtxArray;
    std::map<int, int> vtxMap;

    for (; !itPoly.isDone(); itPoly.next()) {
        itPoly.numTriangles(numTriangles);
        itPoly.getVertices(vtxArray);

        numVertices = vtxArray.length();
        for (int i = 0; i < numVertices; i++) {
            vtxMap[vtxArray[i]] = i;
        }

        MPointArray pointArray;
        MIntArray intArray;

        for (int i = 0; i < numTriangles; i++) {
            // Each triangles
            UVPoint uvPointArray[3];

            itPoly.getTriangle(i, pointArray, intArray);
            float u;
            float v;
            int uvId;
            for (unsigned int n = 0; n < intArray.length(); n++) {
                int localIndex = vtxMap[intArray[n]];
                itPoly.getUVIndex(localIndex, uvId, u, v);
                UVPoint point(u, v);
                uvPointArray[n] = point;
            }

            float& Ax = uvPointArray[0].u;
            float& Ay = uvPointArray[0].v;
            float& Bx = uvPointArray[1].u;
            float& By = uvPointArray[1].v;
            float& Cx = uvPointArray[2].u;
            float& Cy = uvPointArray[2].v;

            float area = ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) / 2;

            if (area < 0) {
                MString index;
                index.set(itPoly.index());
                MString s = mDagPath.fullPathName() + ".f[" + index + "]";
                resultArray.append(s);
            }
        }
        vtxMap.clear();
    }

    MPxCommand::setResult(resultArray);
    return MS::kSuccess;
}

MStatus UvChecker::undoIt()
{
    return MS::kSuccess;
}

bool UvChecker::isUndoable() const
{
    return false;
}

void* UvChecker::creater()
{
    return new UvChecker;
}
