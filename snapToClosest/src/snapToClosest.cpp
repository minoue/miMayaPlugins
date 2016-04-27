//
//  snapToClosest.cpp
//  SnapToClosest
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
#include <maya/MGlobal.h>
#include <maya/MArgList.h>
#include <maya/MItMeshVertex.h>
#include <maya/MIntArray.h>
#include <maya/MPoint.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatVector.h>
#include <maya/MVector.h>
#include <maya/MArgDatabase.h>


using namespace std;


// Flags for this command
static const char * modeFlag            = "-m";
static const char * modeFlagLong        = "-mode";
static const char * distanceFlag        = "-sd";
static const char * distanceFlagLong    = "-searchDistance";


// Constructor   
SnapToClosest::SnapToClosest() : 
    dummyBool(true),
    searchDistance(10)
{}


// Destructor
SnapToClosest::~SnapToClosest() {
}


// Syntax for the command arguments
MSyntax SnapToClosest::newSyntax() {

    MSyntax syntax;
    MStatus status;

    // dummy flag, not to be used.
    syntax.addFlag("-du", "-dummy", MSyntax::kBoolean);

    status = syntax.addFlag(modeFlag, modeFlagLong, MSyntax::kString);
    CHECK_MSTATUS_AND_RETURN(status, syntax);

    status = syntax.addFlag(distanceFlag, distanceFlagLong, MSyntax::kDouble);
    CHECK_MSTATUS_AND_RETURN(status, syntax);

    syntax.enableEdit(false);
    syntax.enableQuery(false);

    return syntax;
}


MStatus SnapToClosest::doIt( const MArgList& args)
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

    // Iteration start
    for ( ; !vIter.isDone(); vIter.next() ) {

        // Curent point position in MPoint
        MPoint currentPosition = vIter.position(MSpace::kWorld);
        MPoint closestPoint;

        // MVector to check the distance between current point and target point
        MVector distanceVector;     


        //current vertex index 
        int currentIndex = vIter.index();

        // polygon face ID 
        int faceIndex;

        fnMeshTarget.getClosestPoint(
            currentPosition,
            closestPoint,
            MSpace::kWorld,
            &faceIndex);

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
            for (unsigned int i=0; i<faceVertexArray.length(); i++){
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
        } else if ( mode == "normal" ) {
            MVector normalVector;
            vIter.getNormal(normalVector, MSpace::kWorld);
            normalVector.normalize();

            MFloatPoint         raySource(currentPosition);
            MFloatVector        rayDirection(normalVector);
            MFloatPoint         hitPoint;

            fnMeshTarget.closestIntersection(
                raySource,
                rayDirection,
                0,
                0,
                false,
                MSpace::kWorld,
                9999,
                false,
                0,
                hitPoint,
                0,
                0,
                0,
                0,
                0);

            MPoint mHitPoint(hitPoint.x, hitPoint.y, hitPoint.z);
            distanceVector = mHitPoint - currentPosition;

            if ( distanceVector.length() < searchDistance ) {
                newVertexArray[currentIndex] = mHitPoint;
            }
        }

        // In surface mode, simply add the closest point to the array
        else if ( mode == "surface") {
            distanceVector = closestPoint - currentPosition;
            if ( distanceVector.length() < searchDistance ) {
                newVertexArray[currentIndex] = closestPoint;
            }
        }
        else {
            MGlobal::displayError("*** WRONG MODE. USE 'vertex', 'normal', or 'surface'. ***");
            return MStatus::kFailure;
        }
    }

    fnMeshComponents.setPoints(newVertexArray, MSpace::kWorld);
    return status;
}


MStatus SnapToClosest::undoIt() {
    fnMeshComponents.setPoints(vertexArray, MSpace::kWorld);
    return MS::kSuccess;
}

bool SnapToClosest::isUndoable() const {
    return true;
}

void* SnapToClosest::creator()
{
    return new SnapToClosest;
}                                           
