//
// snapToClosest.cpp
// SnapToClosest
//
//
// ----------------------------------------------------------------------------
// "THE BEER-WARE LICENSE" (Revision 42):
// <michitaka_inoue@icloud.com> wrote this file.  As long as you retain this notice you
// can do whatever you want with this stuff. If we meet some day, and you think
// this stuff is worth it, you can buy me a beer in return.   Michitaka Inoue
// ----------------------------------------------------------------------------
//

#include "snapToClosest.h"
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatVector.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MItMeshVertex.h>
#include <maya/MMatrix.h>
#include <maya/MMeshIntersector.h>
#include <maya/MPoint.h>

using namespace std;

// Flags for this command
static const char* modeFlag = "-m";
static const char* modeFlagLong = "-mode";
static const char* searchRadiusFlag = "-r";
static const char* searchRadiusFlagLong = "-searchRadius";
static const char* customVectorFlag = "-cv";
static const char* customVectorFlagLong = "-customVector";
static const char* customVectorFlagX = "-cvx";
static const char* customVectorFlagXLong = "-customVectorX";
static const char* customVectorFlagY = "-cvy";
static const char* customVectorFlagYLong = "-customVectorY";
static const char* customVectorFlagZ = "-cvz";
static const char* customVectorFlagZLong = "-customVectorZ";
static const char* testBothDirectionsFlag = "-tbd";
static const char* testBothDirectionsFlagLong = "-testBothDirections";

SnapToClosest::SnapToClosest()
    : dummyBool(true)
    , searchRadius(1000)
{
}

SnapToClosest::~SnapToClosest()
{
}

MSyntax SnapToClosest::newSyntax()
{

    MSyntax syntax;
    MStatus status;

    // dummy flag, not to be used.
    syntax.addFlag("-du", "-dummy", MSyntax::kBoolean);

    status = syntax.addFlag(modeFlag, modeFlagLong, MSyntax::kString);
    CHECK_MSTATUS_AND_RETURN(status, syntax);

    status = syntax.addFlag(searchRadiusFlag, searchRadiusFlagLong, MSyntax::kDouble);
    CHECK_MSTATUS_AND_RETURN(status, syntax);

    syntax.addFlag(customVectorFlag, customVectorFlagLong, MSyntax::kBoolean);
    syntax.addFlag(customVectorFlagX, customVectorFlagXLong, MSyntax::kDouble);
    syntax.addFlag(customVectorFlagY, customVectorFlagYLong, MSyntax::kDouble);
    syntax.addFlag(customVectorFlagZ, customVectorFlagZLong, MSyntax::kDouble);
    syntax.addFlag(testBothDirectionsFlag, testBothDirectionsFlagLong, MSyntax::kBoolean);

    syntax.enableEdit(false);
    syntax.enableQuery(false);

    return syntax;
}

MStatus SnapToClosest::doIt(const MArgList& args)
{
    MStatus status;

    if (args.length() != 1) {
        MGlobal::displayError("Need 2 argument.\n Target mesh and max searchRadius!\n eg. cmds.snapToClosest('|pSphere1', 100)\n");
        return MStatus::kFailure;
    }

    // Read command flags from MSyntax
    MArgDatabase argData(newSyntax(), args, &status);
    argData.getFlagArgument("-du", 0, dummyBool);
    argData.getFlagArgument("-m", 0, mode);
    argData.getFlagArgument(searchRadiusFlag, 0, searchRadius);

    // Read snap target object from args
    targetObjectName = args.asString(0, &status);

    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to read arguments");
        return MStatus::kFailure;
    }
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (argData.isFlagSet(customVectorFlagLong)) {
        argData.getFlagArgument(customVectorFlagLong, 0, useCustomVector);
    } else {
        useCustomVector = false;
    }

    if (argData.isFlagSet(testBothDirectionsFlag)) {
        argData.getFlagArgument(testBothDirectionsFlagLong, 0, testBothDirections);
    } else {
        testBothDirections = false;
    }

    if (useCustomVector == true) {
        if (argData.isFlagSet(customVectorFlagXLong)) {
            argData.getFlagArgument(customVectorFlagXLong, 0, customVectorX);
        } else {
            MGlobal::displayInfo("No x is given");
            return MS::kFailure;
        }

        if (argData.isFlagSet(customVectorFlagYLong)) {
            argData.getFlagArgument(customVectorFlagYLong, 0, customVectorY);
        } else {
            MGlobal::displayInfo("No y is given");
            return MS::kFailure;
        }

        if (argData.isFlagSet(customVectorFlagZLong)) {
            argData.getFlagArgument(customVectorFlagZLong, 0, customVectorZ);
        } else {
            MGlobal::displayInfo("No z is given");
            return MS::kFailure;
        }
        customVector.x = customVectorX;
        customVector.y = customVectorY;
        customVector.z = customVectorZ;
    }

    MGlobal::getActiveSelectionList(mList);

    // If multiple objects are selected, fails
    if (mList.length() != 1) {
        MGlobal::displayError("Multiple objects are selected. Select");
        return MStatus::kFailure;
    }

    return redoIt();
}

MStatus SnapToClosest::redoIt()
{
    MStatus status;

    MDagPath sourceDagPath;
    MDagPath targetDagPath;
    MObject components;

    mList.add(targetObjectName);
    mList.getDagPath(0, sourceDagPath, components);
    mList.getDagPath(1, targetDagPath);

    // Store current point positions for undo
    fnMeshComponents.setObject(sourceDagPath);
    fnMeshComponents.getPoints(oldPositions, MSpace::kWorld);

    // Store curent point position for new positions.
    fnMeshComponents.getPoints(newPositions, MSpace::kWorld);

    // Create mesh vertex iterator
    MItMeshVertex vIter(sourceDagPath, components);

    if (mode == "normal") {
        snapToClosestNormal(sourceDagPath, components, targetDagPath);
    }
    else if (mode == "surface") {
        snapToClosestSurface(sourceDagPath, components, targetDagPath);
    }
    else if (mode == "vertex") {
        snapToClosestVertex(sourceDagPath, components, targetDagPath);
    }

    fnMeshComponents.setPoints(newPositions, MSpace::kWorld);
    return MS::kSuccess;
}

MStatus SnapToClosest::snapToClosestNormal(MDagPath& sourceDagPath, MObject& components, MDagPath& targetDagPath) {

    MFnMesh target(targetDagPath);

    for (MItMeshVertex vIter(sourceDagPath, components); !vIter.isDone(); vIter.next()) {
        MPoint currentPosition = vIter.position(MSpace::kWorld);
        int currentIndex = vIter.index();

        // Setup intersector
        MVector normalVector;
        if (useCustomVector) {
            normalVector.x = customVectorX;
            normalVector.y = customVectorY;
            normalVector.z = customVectorZ;
        } else {
            vIter.getNormal(normalVector, MSpace::kWorld);
        }
        normalVector.normalize();

        MFloatPoint raySource(currentPosition);
        MFloatVector rayDirection(normalVector);
        MFloatPoint hitPoint;

        target.closestIntersection(
            raySource,
            rayDirection,
            0,
            0,
            false,
            MSpace::kWorld,
            999999,
            testBothDirections,
            0,
            hitPoint,
            0,
            0,
            0,
            0,
            0);

        MPoint mHitPoint(hitPoint.x, hitPoint.y, hitPoint.z);
        MVector distanceVector = mHitPoint - currentPosition;

        if (distanceVector.length() < searchRadius) {
            newPositions[currentIndex] = mHitPoint;
        }
    }
    return MS::kSuccess;
}

MStatus SnapToClosest::snapToClosestSurface(MDagPath& sourceDagPath, MObject& components, MDagPath& targetDagPath) {

    // Setup intersector
    MMatrix matrix = targetDagPath.inclusiveMatrix();
    MMeshIntersector intersector;
    targetDagPath.extendToShape();
    MObject targetMObj = targetDagPath.node();
    intersector.create(targetMObj, matrix);

    for (MItMeshVertex vIter(sourceDagPath, components); !vIter.isDone(); vIter.next()) {

        // Curent point position in MPoint
        MPoint currentPosition = vIter.position(MSpace::kWorld);

        //current vertex index
        int currentIndex = vIter.index();

        MPointOnMesh pointInfo;
        intersector.getClosestPoint(currentPosition, pointInfo);

        MPoint closestPoint = pointInfo.getPoint();

        // This is the point of snap target
        closestPoint = closestPoint * matrix;

        MVector distanceVector = closestPoint - currentPosition;
        if (distanceVector.length() < searchRadius) {
            newPositions[currentIndex] = closestPoint;
        }
    }
    return MS::kSuccess;
}

MStatus SnapToClosest::snapToClosestVertex(MDagPath& sourceDagPath, MObject& components, MDagPath& targetDagPath) {
    MFnMesh target(targetDagPath);

    // Setup intersector
    MMatrix matrix = targetDagPath.inclusiveMatrix();
    MMeshIntersector intersector;
    targetDagPath.extendToShape();
    MObject targetMObj = targetDagPath.node();
    intersector.create(targetMObj, matrix);

    for (MItMeshVertex vIter(sourceDagPath, components); !vIter.isDone(); vIter.next()) {

        // Curent point position
        MPoint currentPosition = vIter.position(MSpace::kWorld);

        //current vertex index
        int currentIndex = vIter.index();

        // Value to check
        double shortestDistance = 10000;

        MPointOnMesh pointInfo;
        intersector.getClosestPoint(currentPosition, pointInfo);
        int faceIndex = pointInfo.faceIndex();

        // MPoint closestVertex = currentPosition;
        MPoint closestVertex = currentPosition;

        // Get face index array
        MIntArray faceVertexArray;
        target.getPolygonVertices(faceIndex, faceVertexArray);

        // Loop face vertices and find a vertex which is closest to the
        // current vertex.
        for (unsigned int i = 0; i < faceVertexArray.length(); i++) {
            int vertexIndex = faceVertexArray[i];
            MPoint pointPosition;
            target.getPoint(vertexIndex, pointPosition, MSpace::kWorld);

            MVector distanceVector = pointPosition - currentPosition;

            // If length is shorter than search distance, keep current point.
            if (distanceVector.length() < searchRadius) {
                if (distanceVector.length() < shortestDistance) {
                    shortestDistance = distanceVector.length();
                    closestVertex = pointPosition;
                }
            }
        }
        // Set closest vertex to new vertex array
        newPositions[currentIndex] = closestVertex;

        // In normal mode, add cloest point along normal vector.
    }
    return MS::kSuccess;
}

MStatus SnapToClosest::undoIt()
{
    fnMeshComponents.setPoints(oldPositions, MSpace::kWorld);
    return MS::kSuccess;
}

bool SnapToClosest::isUndoable() const
{
    return true;
}

void* SnapToClosest::creator()
{
    return new SnapToClosest;
}
