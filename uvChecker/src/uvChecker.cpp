#include "uvChecker.h"
#include <math.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MSelectionList.h>
#include <maya/MStringArray.h>
#include <set>

UvChecker::UvChecker()
{
}

UvChecker::~UvChecker()
{
}

MSyntax UvChecker::newSyntax()
{
    MSyntax syntax;
    syntax.addArg(MSyntax::kString);
    syntax.addFlag("-v", "-verbose", MSyntax::kBoolean);
    syntax.addFlag("-c", "-check", MSyntax::kUnsigned);
    syntax.addFlag("-uva", "-uvArea", MSyntax::kDouble);
    return syntax;
}

MStatus UvChecker::doIt(const MArgList& args)
{
    MStatus status;
    
    MSelectionList sel;

    MArgDatabase argData(syntax(), args);

    status = argData.getCommandArgument(0, sel);
    if (status != MS::kSuccess) {
        MGlobal::displayError("You have to provide an object path");
        return MStatus::kFailure;
    }

    if (argData.isFlagSet("-verbose"))
        argData.getFlagArgument("-verbose", 0, verbose);
    else
        verbose = false;

    if (argData.isFlagSet("-check"))
        argData.getFlagArgument("-check", 0, checkNumber);
    else
        checkNumber = 99;

    if (argData.isFlagSet("-uvArea"))
        argData.getFlagArgument("-uvArea", 0, minUVArea);
    else
        minUVArea = 0.000001;

    sel.getDagPath(0, mDagPath);

    if (verbose == true) {
        MString objectPath = "Selected mesh : " + mDagPath.fullPathName();
        MGlobal::displayInfo(objectPath);
    }

    status = mDagPath.extendToShape();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (mDagPath.apiType() != MFn::kMesh) {
        MGlobal::displayError("Selected object is not mesh.");
        return MStatus::kFailure;
    }

    return redoIt();
}

MStatus UvChecker::redoIt()
{
    MStatus status;

    switch (checkNumber) {
    case UvChecker::UDIM:
        if (verbose == true) {
            MGlobal::displayInfo("Checking UDIM borders");
        }
        status = findUdimIntersections();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        break;
    case UvChecker::HAS_UVS:
        if (verbose == true) {
            MGlobal::displayInfo("Checking Non UVed faces");
        }
        status = findNoUvFaces();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        break;
    case UvChecker::ZERO_AREA:
        if (verbose == true) {
            MGlobal::displayInfo("Checking Zero UV faces");
        }
        status = findZeroUvFaces();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        break;
    default:
        MGlobal::displayError("Invalid check number");
        return MS::kFailure;
        break;
    }

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

void* UvChecker::creator()
{
    return new UvChecker;
}

MStatus UvChecker::findUdimIntersections()
{
    MStatus status;

    MIntArray indexArray;
    MFnMesh fnMesh(mDagPath);

    std::set<int> indexSet;

    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {

        int vCount = mItPoly.polygonVertexCount();
        int currentUVindex;
        int nextUVindex;
        float u1, v1, u2, v2;

        for (int i = 0; i < vCount; i++) {
            mItPoly.getUVIndex(i, currentUVindex);

            if (i == vCount - 1) {
                mItPoly.getUVIndex(0, nextUVindex);
            } else {
                mItPoly.getUVIndex(i + 1, nextUVindex);
            }

            fnMesh.getUV(currentUVindex, u1, v1);
            fnMesh.getUV(nextUVindex, u2, v2);

            if (floor(u1) == floor(u2) && floor(v1) == floor(v2)) {
            } else {
                indexSet.insert(currentUVindex);
                indexSet.insert(nextUVindex);
            }
        }
    }

    std::set<int>::iterator indexSetIter;
    for (indexSetIter = indexSet.begin(); indexSetIter != indexSet.end(); ++indexSetIter) {
        indexArray.append(*indexSetIter);
    }
    
    unsigned int arrayLength = indexArray.length();
    MStringArray resultArray;
    for (unsigned int i = 0; i < arrayLength; i++) {
        MString index;
        index.set(indexArray[i]);
        MString s = mDagPath.fullPathName() + ".map[" + index + "]";
        resultArray.append(s);
    }
    MPxCommand::setResult(resultArray);

    return MS::kSuccess;
}

MStatus UvChecker::findNoUvFaces()
{
    MStringArray resultArray;

    bool hasUVs;
    for (MItMeshPolygon itPoly(mDagPath); !itPoly.isDone(); itPoly.next()) {
        hasUVs = itPoly.hasUVs();
        if (hasUVs == false) {
            MString index;
            index.set(itPoly.index());
            MString s = mDagPath.fullPathName() + ".f[" + index + "]";
            resultArray.append(s);
        }
    }
    MPxCommand::setResult(resultArray);
    return MS::kSuccess;
}

MStatus UvChecker::findZeroUvFaces()
{
    MStringArray resultArray;
    double area;
    bool hasUVs;
    MString index;

    MString temp;
    temp.set(minUVArea);
    MGlobal::displayInfo(temp);

    for (MItMeshPolygon itPoly(mDagPath); !itPoly.isDone(); itPoly.next()) {
        hasUVs = itPoly.hasUVs();
        if (hasUVs == false) {
        } else {
            itPoly.getUVArea(area);
            temp.set(area);
            // MGlobal::displayInfo(temp);
            if (area < minUVArea) {
                index.set(itPoly.index());
                MString s = mDagPath.fullPathName() + ".f[" + index + "]";
                resultArray.append(s);
            }
        }
    }
    MPxCommand::setResult(resultArray);
    return MS::kSuccess;
}
