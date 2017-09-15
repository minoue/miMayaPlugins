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

MStatus TopologyChecker::findTriangles(MItMeshPolygon& mItPoly, MIntArray& indexArray)
{
    MStatus status;
    for (; !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.polygonVertexCount() == 3) {
            indexArray.append(mItPoly.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findNgons(MItMeshPolygon& mItPoly, MIntArray& indexArray)
{
    MStatus status;
    for (; !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.polygonVertexCount() >= 5) {
            indexArray.append(mItPoly.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findNonManifoldEdges(MItMeshEdge& mItEdge, MIntArray& indexArray)
{
    MStatus status;
    int faceCount;
    for (; !mItEdge.isDone(); mItEdge.next()) {
        status = mItEdge.numConnectedFaces(faceCount);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        std::cout << faceCount << std::endl;
        if (faceCount >= 3) {
            indexArray.append(mItEdge.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findLaminaFaces(MItMeshPolygon& mItPoly, MIntArray& indexArray)
{
    MStatus status;
    for (; !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.isLamina() == true) {
            indexArray.append(mItPoly.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findBiValentFaces(MItMeshVertex& mItVert, MIntArray& indexArray)
{
    MStatus status;
    MIntArray connectedFaces;
    MIntArray connectedEdges;
    for (; !mItVert.isDone(); mItVert.next()) {
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

MStatus TopologyChecker::findZeroAreaFaces(MItMeshPolygon& mItPoly, MIntArray& indexArray, double& faceAreaMax)
{
    MStatus status;
    double area;
    for (; !mItPoly.isDone(); mItPoly.next()) {
        mItPoly.getArea(area);
        if (area < faceAreaMax) {
            indexArray.append(mItPoly.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findZeroAreaUV(MItMeshPolygon& mItPoly,
    MIntArray& indexArray,
    double& uvAreaMax)
{
    MStatus status;
    double area;
    for (; !mItPoly.isDone(); mItPoly.next()) {
        mItPoly.getUVArea(area);
        if (area < uvAreaMax) {
            indexArray.append(mItPoly.index());
        }
    }
    return status;
}

MStatus TopologyChecker::findUdimIntersections(const MDagPath& dagPath,
    MIntArray& indexArray)
{
    MStatus status;

    MFnMesh fnMesh(dagPath);

    std::set<int> indexSet;

    // clock_t clock_start = clock();
    for (MItMeshPolygon mItPoly(dagPath); !mItPoly.isDone(); mItPoly.next()) {

        int vCount = mItPoly.polygonVertexCount();
        int currentIndex;
        int nextIndex;
        float u1, v1, u2, v2;

        for (int i = 0; i < vCount; i++) {
            mItPoly.getUVIndex(i, currentIndex);

            if (i == vCount - 1) {
                mItPoly.getUVIndex(0, nextIndex);
            } else {
                mItPoly.getUVIndex(i + 1, nextIndex);
            }

            fnMesh.getUV(currentIndex, u1, v1);
            fnMesh.getUV(nextIndex, u2, v2);

            if (floor(u1) == floor(u2) && floor(v1) == floor(v2)) {
            } else {
                indexSet.insert(currentIndex);
                indexSet.insert(nextIndex);
            }
        }
    }

    // double time;
    // time = 1000.0*static_cast<double>(clock()- clock_start) / CLOCKS_PER_SEC;
    // MString timeStr;
    // timeStr.set(time);
    // MGlobal::displayInfo(timeStr);

    std::set<int>::iterator indexSetIter;
    for (indexSetIter = indexSet.begin(); indexSetIter != indexSet.end(); ++indexSetIter) {
        indexArray.append(*indexSetIter);
    }

    return MS::kSuccess;
}

MStatus TopologyChecker::findUvOverlaps(const MDagPath& dagPath, MIntArray& indexArray)
{
    MStatus status;

    MFnMesh fnMesh(dagPath);

    std::set<UVEdge> edgeSet;

    for (MItMeshPolygon mItPoly(dagPath); !mItPoly.isDone(); mItPoly.next()) {

        int vCount = mItPoly.polygonVertexCount();
        int currentIndex;
        int nextIndex;
        float u1, v1, u2, v2;

        for (int i = 0; i < vCount; i++) {
            mItPoly.getUVIndex(i, currentIndex);

            if (i == vCount - 1) {
                mItPoly.getUVIndex(0, nextIndex);
            } else {
                mItPoly.getUVIndex(i + 1, nextIndex);
            }

            fnMesh.getUV(currentIndex, u1, v1);
            fnMesh.getUV(nextIndex, u2, v2);

            UVPoint uvp1(u1, v1, currentIndex);
            UVPoint uvp2(u2, v2, nextIndex);
            UVEdge edge(uvp1, uvp2);
            edgeSet.insert(edge);
        }
    }

    std::set<UVEdge>::iterator edgeSetIter;
    for (edgeSetIter = edgeSet.begin(); edgeSetIter != edgeSet.end(); ++edgeSetIter) {
    }

    return MS::kSuccess;
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
    syntax.addFlag("-a", "-uvAreaMax", MSyntax::kDouble);
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

    if (argData.isFlagSet("-uvAreaMax"))
        argData.getFlagArgument("-uvAreaMax", 0, uvAreaMax);
    else
        uvAreaMax = 0.00001;

    if (argData.isFlagSet("-faceAreaMax"))
        argData.getFlagArgument("-faceAreaMax", 0, faceAreaMax);
    else
        faceAreaMax = 0.00001;

    MSelectionList mList;
    mList.add(argument);
    mList.getDagPath(0, mDagPath);

    MStringArray resultArray;
    MIntArray indexArray;

    MItMeshPolygon mItPoly(mDagPath);
    MItMeshEdge mItEdge(mDagPath);
    MItMeshVertex mItVert(mDagPath);

    switch (checkNumber) {
    case TopologyChecker::TRIANGLES:
        status = findTriangles(mItPoly, indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "face");
        break;
    case TopologyChecker::NGONS:
        status = findNgons(mItPoly, indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "face");
        break;
    case TopologyChecker::NON_MANIFOLD_EDGES:
        status = findNonManifoldEdges(mItEdge, indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "edge");
        break;
    case TopologyChecker::LAMINA_FACES:
        status = findLaminaFaces(mItPoly, indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "face");
        break;
    case TopologyChecker::BI_VALENT_FACES:
        status = findBiValentFaces(mItVert, indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "vertex");
        break;
    case TopologyChecker::ZERO_AREA_FACES:
        status = findZeroAreaFaces(mItPoly, indexArray, faceAreaMax);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "face");
        break;
    case TopologyChecker::ZERO_AREA_UV:
        status = findZeroAreaUV(mItPoly, indexArray, uvAreaMax);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "face");
        break;
    case TopologyChecker::UDIM:
        status = findUdimIntersections(mDagPath, indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "uv");
        break;
    case TopologyChecker::UV_OVERLAP:
        status = findUvOverlaps(mDagPath, indexArray);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString(indexArray, "uv");
        break;
    case TopologyChecker::TEST:
        break;
    default:
        MGlobal::displayError("Invalid check number");
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
