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
static const char* distanceFlag = "-d";
static const char* distanceFlagLong = "-searchDistance";
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
    , searchDistance(1000)
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

    status = syntax.addFlag(distanceFlag, distanceFlagLong, MSyntax::kDouble);
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
        MGlobal::displayError("Need 2 argument.\n Target mesh and max searchDistance!\n eg. cmds.snapToClosest('|pSphere1', 100)\n");
        return MStatus::kFailure;
    }

    // Read command flags from MSyntax
    MArgDatabase argData(newSyntax(), args, &status);
    argData.getFlagArgument("-du", 0, dummyBool);
    argData.getFlagArgument("-m", 0, mode);
    argData.getFlagArgument(distanceFlag, 0, searchDistance);

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

    mList.add(targetObjectName);
    mList.getDagPath(0, mDagPath_components, components);
    mList.getDagPath(1, mDagPath_target);

    // Store current point positions for undo
    fnMeshComponents.setObject(mDagPath_components);
    fnMeshComponents.getPoints(vertexArray, MSpace::kWorld);

    // Store curent point position for new.
    fnMeshComponents.getPoints(newVertexArray, MSpace::kWorld);

    // Create fnMesh for target mesh
    fnMeshTarget.setObject(mDagPath_target);

    // Create mesh vertex iterator
    MItMeshVertex vIter(mDagPath_components, components);

    // Setup intersector
    MMatrix matrix = mDagPath_target.inclusiveMatrix();
    MMeshIntersector intersector;
    mDagPath_target.extendToShape();
    MObject targetMObj = mDagPath_target.node();
    intersector.create(targetMObj, matrix);

    // Iteration start
    for (; !vIter.isDone(); vIter.next()) {

        // Curent point position in MPoint
        MPoint currentPosition = vIter.position(MSpace::kWorld);
        MPoint closestPoint;

        // MVector to check the distance between current point and target point
        MVector distanceVector;

        //current vertex index
        int currentIndex = vIter.index();

        // polygon face ID
        int faceIndex;

        MPointOnMesh pointInfo;

        intersector.getClosestPoint(
            currentPosition,
            pointInfo);

        faceIndex = pointInfo.faceIndex();
        closestPoint = pointInfo.getPoint();
        // Apply matrix of target mesh
        closestPoint = closestPoint * matrix;

        // If vertex mode, snap to closest verteces.
        if (mode == "vertex") {
            // Get closest face 's vertices
            MIntArray faceVertexArray;

            // Value to check
            double shortestDistance = 10000;

            // MPoint closestVertex = currentPosition;
            MPoint closestVertex = currentPosition;

            // Get face index array
            fnMeshTarget.getPolygonVertices(faceIndex, faceVertexArray);

            // Loop face vertices and find a vertex which is closest to the
            // current vertex.
            for (unsigned int i = 0; i < faceVertexArray.length(); i++) {
                int vertexIndex = faceVertexArray[i];
                MPoint pointPosition;
                fnMeshTarget.getPoint(vertexIndex, pointPosition, MSpace::kWorld);

                distanceVector = pointPosition - currentPosition;

                // If length is shorter than search distance, keep current point.
                if (distanceVector.length() < searchDistance) {
                    if (distanceVector.length() < shortestDistance) {
                        shortestDistance = distanceVector.length();
                        closestVertex = pointPosition;
                    }
                }
            }
            // Set closest vertex to new vertex array
            newVertexArray[currentIndex] = closestVertex;

            // In normal mode, add cloest point along normal vector.
        } else if (mode == "normal") {
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

            fnMeshTarget.closestIntersection(
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
            distanceVector = mHitPoint - currentPosition;

            if (distanceVector.length() < searchDistance) {
                newVertexArray[currentIndex] = mHitPoint;
            }
        }

        // In surface mode, simply add the closest point to the array
        else if (mode == "surface") {
            distanceVector = closestPoint - currentPosition;
            if (distanceVector.length() < searchDistance) {
                newVertexArray[currentIndex] = closestPoint;
            }
        } else {
            MGlobal::displayError("*** WRONG MODE. USE 'vertex', 'normal', or 'surface'. ***");
            return MStatus::kFailure;
        }
    }

    fnMeshComponents.setPoints(newVertexArray, MSpace::kWorld);
    return status;
}

MStatus SnapToClosest::undoIt()
{
    fnMeshComponents.setPoints(vertexArray, MSpace::kWorld);
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
