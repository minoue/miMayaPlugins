#include "symmetryTable.h"
#include <maya/MGlobal.h>
#include <maya/MArgList.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MFnMesh.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MItMeshVertex.h>
#include <maya/MIntArray.h>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <maya/MMatrix.h>
#include <maya/MMeshIntersector.h>


SymmetryTable::SymmetryTable() {
}

SymmetryTable::~SymmetryTable() {
}

MStatus SymmetryTable::doIt(const MArgList& args)
{
    MStatus status;

    MSelectionList mList;
    MGlobal::getActiveSelectionList(mList);
	MDagPath dagPath;
	MObject components;

	mList.getDagPath(0, dagPath);
	MFnMesh fnMesh(dagPath);

	MIntArray indices;

	MPointArray pointArray;
	fnMesh.getPoints(pointArray);
	int numPoints = pointArray.length();

	for (int i = 0; i < numPoints; i++) {
		MPoint p = pointArray[i];
		if (p.x > 0.0) {
			indices.append(i);
		}
	}

    MString command;
    std::string cmd("select -r ");
    for (int i = 0; i < indices.length(); i++) {
    	std::string path = dagPath.fullPathName().asChar();
    	std::string fullPath = path + ".vtx[" + std::to_string(indices[i]) + "] ";
        cmd.append(fullPath);
    }

    MString a;
    a.set(cmd.c_str());
    MGlobal::executeCommand(a);

	MGlobal::getActiveSelectionList(mList);
    mList.getDagPath(0, dagPath, components);
 
    MMatrix matrix = dagPath.inclusiveMatrix();
    MMeshIntersector intersector;
    dagPath.extendToShape();
    MObject obj = dagPath.node();
    intersector.create(obj, matrix);

    MPoint closestPoint;
    double shortestDistance;
    int faceIndex;
 
    std::vector<std::pair<int, int>> vec;
 
    for (MItMeshVertex itVerts(dagPath, components); !itVerts.isDone(); itVerts.next()) {
        MPoint oppositePoint = itVerts.position();
        oppositePoint.x = -oppositePoint.x;

        MPointOnMesh pointInfo;

        intersector.getClosestPoint(oppositePoint, pointInfo);

        faceIndex = pointInfo.faceIndex();
        closestPoint = pointInfo.getPoint();
        closestPoint = closestPoint * matrix;

        // fnMesh.getClosestPoint(oppositePoint, closestPoint, MSpace::kObject, &faceIndex);
        MIntArray faceVertices;
        fnMesh.getPolygonVertices(faceIndex, faceVertices);
 
        int closestVertex;
        for (unsigned int i = 0; i < faceVertices.length(); i++) {
            MPoint vertexPoint;
            fnMesh.getPoint(faceVertices[i], vertexPoint);
            MVector v = vertexPoint - oppositePoint;
            double length = v.length();
            if (i == 0) {
                // first loop
                closestVertex = faceVertices[i];
                shortestDistance = length;
            }
            else {
                if (length < shortestDistance) {
                    closestVertex = faceVertices[i];
                    shortestDistance = length;
                }
                else {
                }
            }
        }
        int a = itVerts.index();
        int b = closestVertex;
        if (a != b) {
            vec.push_back(std::make_pair(a, b));
        }
    }
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
 
    MIntArray re;
    for (int i = 0; i < vec.size(); i++) {
        re.append(vec[i].first);
        re.append(vec[i].second);
    }
    setResult(re);

    return MS::kSuccess;
    // return redoIt();
}

MStatus SymmetryTable::redoIt() {
    MStatus status;
    return status;
}

MStatus SymmetryTable::undoIt() {
    return MS::kSuccess;
}

bool SymmetryTable::isUndoable() const {
    return false;
}

void* SymmetryTable::creater()
{
    return new SymmetryTable;
}                                           
