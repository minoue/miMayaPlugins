#include "randomPointOnMesh.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime>
#include <math.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MColor.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatArray.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MStatus.h>


#define NUMBER_OF_POINTS 1000


// Flags for this command
static const char * numberFlag            = "-n";
static const char * numberFlagLong        = "-numOfPoints";
static const char * colorSetFlag          = "-cs";
static const char * colorSetFlagLong      = "-colorSet";



RandomPointOnMesh::RandomPointOnMesh() {
    numOfPoints = NUMBER_OF_POINTS;
    colorSet = "";
}


RandomPointOnMesh::~RandomPointOnMesh() {}


// Syntax for the command arguments
MSyntax RandomPointOnMesh::newSyntax() {

    MSyntax syntax;
    MStatus status;

    status = syntax.addFlag(numberFlag, numberFlagLong, MSyntax::kUnsigned);
    CHECK_MSTATUS_AND_RETURN(status, syntax);

    status = syntax.addFlag(colorSetFlag, colorSetFlagLong, MSyntax::kString);
    CHECK_MSTATUS_AND_RETURN(status, syntax);

    syntax.enableEdit(false);
    syntax.enableQuery(false);

    return syntax;
}


MStatus RandomPointOnMesh::getWeight(MFloatArray& weightArray)
{
    MStatus status;

    int numOfFaces = fnMesh.numPolygons();
    // weightArray.setLength(numOfFaces);
    MItMeshPolygon faceIt(mDagPath);
    MColor color;
    for ( faceIt.reset(); !faceIt.isDone(); faceIt.next() ){
        if (colorSet == "") {
            weightArray.append(1.0);
        } else {
            faceIt.getColor(color);
            float red = color.r;
            weightArray.append(red);
        }
    }

    return MS::kSuccess;
}

MStatus RandomPointOnMesh::getTwoVectors( const MIntArray& faceIDs,
                                      MPoint& origin,
                                      MVector& v1,
                                      MVector& v2) {
    MStatus status;
    MPoint p2;
    MPoint p3;
    fnMesh.getPoint(faceIDs[0], origin, MSpace::kWorld);
    fnMesh.getPoint(faceIDs[1], p2, MSpace::kWorld);
    fnMesh.getPoint(faceIDs[2], p3, MSpace::kWorld);
    v1 = p2 - origin;
    v2 = p3 - origin;

    return MS::kSuccess;
}


MStatus RandomPointOnMesh::getAccumulatedArea(const std::vector<MIntArray> &v, MDoubleArray& accumulatedArea) {

    MStatus status;

    MPoint originPoint;
    MVector v1;
    MVector v2;

    for (unsigned int i=0; i<v.size(); i++) {

        getTwoVectors(v[i], originPoint, v1, v2);
        MVector cross = v1 ^ v2;
        double area = cross.length() / 2.0;
        if ( i==0 ) {
            accumulatedArea.append(area);
        } else {
            area = area + accumulatedArea[i-1];
            accumulatedArea.append(area);
        }
    }

    return MS::kSuccess;
}

MStatus RandomPointOnMesh::doIt( const MArgList& args)
{
    MStatus status;

    // if (args.length() != 1) {
    //     MGlobal::displayError("Wrong number of arguments");
    //     return MStatus::kFailure;
    // }

    // Read command flags from MSyntax
    MSyntax syntax = newSyntax();
    MArgDatabase argData(syntax, args, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    argData.getFlagArgument("-n", 0, numOfPoints);

    if (argData.isFlagSet(numberFlagLong)) {
        unsigned tmp;
        status = argData.getFlagArgument(numberFlagLong, 0, tmp);
        if (!status) {
            status.perror("num flag parsing failed.");
            return status;
        }
        numOfPoints = tmp;
    }

    if (argData.isFlagSet(colorSetFlagLong)) {
        MString tmp;
        status = argData.getFlagArgument(colorSetFlagLong, 0, tmp);
        if (!status) {
            status.perror("colorset flag parsing failed.");
            return status;
        }
        colorSet = tmp;
    }

    MSelectionList mList;
    MGlobal::getActiveSelectionList(mList);
    mList.getDagPath(0, mDagPath);

    // Check if selected is mesh
    mDagPath.extendToShape();
    MObject apiType = mDagPath.node(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MString apiTypeString(apiType.apiTypeStr());
    if (apiTypeString != "kMesh") {
        MGlobal::displayError("Selected Object is not mesh");
        return MS::kFailure;
    }

    fnMesh.setObject(mDagPath);

    // Set color set to be used
    colorSet = fnMesh.currentColorSetName(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MString colorSetOutput("Current color set : ");
    if (colorSet != "") {
        colorSetOutput += colorSet;
    } else {
        MString tmpStr("empty");
        colorSetOutput += tmpStr;
    }
    MGlobal::displayInfo(colorSetOutput);

    return redoIt();
}

MStatus RandomPointOnMesh::redoIt() {
    MStatus status;

    MFloatArray weightArray;
    getWeight(weightArray);
    
    int localTriCount;

    // Array of triangle vertices
    std::vector<MIntArray> triangleArray;

    MItMeshPolygon faceIt(mDagPath);
    for (faceIt.reset(); !faceIt.isDone(); faceIt.next() ) {
        faceIt.numTriangles(localTriCount);
        if (localTriCount >= 2) {
            MGlobal::displayError("Non-triangle face was found. Make sure all faces are trianglated.");
            return MS::kFailure;
        }

        for (unsigned int i=0; i<localTriCount; i++) {
            MPointArray points;
            MIntArray vertexList;
            faceIt.getTriangle(i, points, vertexList, MSpace::kWorld);
            triangleArray.push_back(vertexList);
        }
    }

    MDoubleArray accumulatedArea;
    getAccumulatedArea(triangleArray, accumulatedArea);

    int numOfTriangles = accumulatedArea.length();
    int areaMax = accumulatedArea[numOfTriangles-1];

    MDoubleArray randomPoints;

    // std::cout << "Number of triangles is " << numOfTriangles << std::endl;
    // std::cout << "Accumulated area is " << areaMax << std::endl;

    // Get random number to find random location on entire surface
    double randomDouble;
    MString cmd = "rand(1, ";
    cmd += areaMax;
    cmd += ");";

    // Get random number to find random location in a triangle
    double randomValue;
    MString cmd2 = "rand(0.0, 1.0);";

    for (unsigned int i=0; i<numOfPoints; i++) {

        MGlobal::executeCommand(cmd, randomDouble);

        for (unsigned int index=0; index<accumulatedArea.length(); index++) {
            if (randomDouble<accumulatedArea[index]) {

                // random value for face
                MGlobal::executeCommand(cmd2, randomValue);
                // std::cout << std::fixed;
                // std::cout << std::setprecision(5) << weightArray[index] << std::endl;
                if (randomValue + weightArray[index] < 1.0) {
                    break;
                }

                MPoint originPoint;
                MVector v1;
                MVector v2;
                getTwoVectors(triangleArray[index], originPoint, v1, v2);
                double u = (double)rand() / RAND_MAX;
                double v = (double)rand() / RAND_MAX;
                if (u + v > 1.0) {
                    u = 1.0 - u;
                    v = 1.0 - v;
                }
                v1 = v1 * u;
                v2 = v2 * v;
                MVector v3 = v1 + v2;
                MPoint newPoint = originPoint + v3;
                randomPoints.append(newPoint[0]);
                randomPoints.append(newPoint[1]);
                randomPoints.append(newPoint[2]);
                break;
            }
        }
    }
    setResult(randomPoints);
    return status;
}

MStatus RandomPointOnMesh::undoIt() {
    return MS::kSuccess;
}

bool RandomPointOnMesh::isUndoable() const {
    return true;
}

void* RandomPointOnMesh::creater()
{
    return new RandomPointOnMesh;
}                                           
