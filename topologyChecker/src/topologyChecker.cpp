//
//  topologyChecker.cpp
//
//

#include "topologyChecker.h"
#include "uvUtils.h"
#include <iostream>
#include <math.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>
#include <set>
#include <sstream>
#include <time.h>

TopologyChecker::TopologyChecker()
{
}

TopologyChecker::~TopologyChecker()
{
}

MStatus TopologyChecker::findTriangles(MIntArray& indexArray)
{
    MStatus status;
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.polygonVertexCount() == 3) {
            indexArray.append(mItPoly.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findNgons(MIntArray& indexArray)
{
    MStatus status;
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.polygonVertexCount() >= 5) {
            indexArray.append(mItPoly.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findNonManifoldEdges(MIntArray& indexArray)
{
    MStatus status;
    int faceCount;
    for (MItMeshEdge mItEdge(mDagPath); !mItEdge.isDone(); mItEdge.next()) {
        status = mItEdge.numConnectedFaces(faceCount);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        std::cout << faceCount << std::endl;
        if (faceCount >= 3) {
            indexArray.append(mItEdge.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findLaminaFaces(MIntArray& indexArray)
{
    MStatus status;
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.isLamina() == true) {
            indexArray.append(mItPoly.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findBiValentFaces(MIntArray& indexArray)
{
    MStatus status;
    MIntArray connectedFaces;
    MIntArray connectedEdges;
    for (MItMeshVertex mItVert(mDagPath); !mItVert.isDone(); mItVert.next()) {
        mItVert.getConnectedFaces(connectedFaces);
        mItVert.getConnectedEdges(connectedEdges);
        int numFaces = connectedFaces.length();
        int numEdges = connectedEdges.length();
        if (numFaces == 2 && numEdges == 2) {
            indexArray.append(mItVert.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findZeroAreaFaces(MIntArray& indexArray, double& faceAreaMax)
{
    MStatus status;
    double area;
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        mItPoly.getArea(area);
        if (area < faceAreaMax) {
            indexArray.append(mItPoly.index());
        }
    }
    return status;
}

MStringArray TopologyChecker::setResultString(MIntArray& indexArray, std::string componentType)
{
    MString fullpath = mDagPath.fullPathName();
    MStringArray resultStringArray;
    int index;
    for (unsigned int i = 0; i < indexArray.length(); i++) {
        index = indexArray[i];
        if (componentType == "face") {
            MString name = fullpath + ".f[" + index + "]";
            resultStringArray.append(name);
        } else if (componentType == "vertex") {
            MString name = fullpath + ".vtx[" + index + "]";
            resultStringArray.append(name);
        } else if (componentType == "edge") {
            MString name = fullpath + ".e[" + index + "]";
            resultStringArray.append(name);
        } else if (componentType == "uv") {
            MString name = fullpath + ".map[" + index + "]";
            resultStringArray.append(name);
        }
    }
    return resultStringArray;
}

MSyntax TopologyChecker::newSyntax()
{
    MSyntax syntax;
    syntax.addFlag("-c", "-check", MSyntax::kUnsigned);
    syntax.addFlag("-fa", "-faceAreaMax", MSyntax::kDouble);
    return syntax;
}

MStatus TopologyChecker::doIt(const MArgList& args)
{
    MStatus status;

    if (args.length() != 1) {
        MGlobal::displayError("Need 1 arg!");
        return MStatus::kFailure;
    }

    MArgDatabase argData(syntax(), args);

    // arg
    MString argument = args.asString(0, &status);
    if (status != MS::kSuccess) {
        return MStatus::kFailure;
    }
    CHECK_MSTATUS_AND_RETURN_IT(status);

    unsigned int checkNumber;
    if (argData.isFlagSet("-check"))
        argData.getFlagArgument("-check", 0, checkNumber);
    else
        checkNumber = 99;

    if (argData.isFlagSet("-faceAreaMax"))
        argData.getFlagArgument("-faceAreaMax", 0, faceAreaMax);
    else
        faceAreaMax = 0.00001;

    MSelectionList mList;
    mList.add(argument);
    mList.getDagPath(0, mDagPath);

    MStringArray resultArray;
    MIntArray indexArray;

    switch (checkNumber) {
    case TopologyChecker::TRIANGLES:
        status = findTriangles(indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "face");
        break;
    case TopologyChecker::NGONS:
        status = findNgons(indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "face");
        break;
    case TopologyChecker::NON_MANIFOLD_EDGES:
        status = findNonManifoldEdges(indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "edge");
        break;
    case TopologyChecker::LAMINA_FACES:
        status = findLaminaFaces(indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "face");
        break;
    case TopologyChecker::BI_VALENT_FACES:
        status = findBiValentFaces(indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "vertex");
        break;
    case TopologyChecker::ZERO_AREA_FACES:
        status = findZeroAreaFaces(indexArray, faceAreaMax);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "face");
        break;
    case TopologyChecker::TEST:
        break;
    default:
        MGlobal::displayError("Invalid check number");
        return MS::kFailure;
        break;
    }

    MPxCommand::setResult(resultArray);

    return redoIt();
}

MStatus TopologyChecker::redoIt()
{
    MStatus status;
    return status;
}

MStatus TopologyChecker::undoIt()
{
    return MS::kSuccess;
}

bool TopologyChecker::isUndoable() const
{
    return false;
}

void* TopologyChecker::creator()
{
    return new TopologyChecker;
}
