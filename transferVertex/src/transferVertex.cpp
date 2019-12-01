#include "transferVertex.h"
#include <maya/MGlobal.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>
#include <maya/MStringArray.h>
#include <maya/MItMeshVertex.h>
#include <maya/MIntArray.h>
#include <maya/MPoint.h>


// Flags for this command
static const char* sourceUvSetFlag = "-suv";
static const char* sourceUvSetFlagLong = "-sourceUvSet";
static const char* targetUvSetFlag = "-tuv";
static const char* targetUvSetFlagLong = "-targetUvSet";
static const char* sourceMeshFlag = "-sm";
static const char* sourceMeshFlagLong = "-sourceMesh";
static const char* targetMeshFlag = "-tm";
static const char* targetMeshFlagLong = "-targetMesh";
static const char* toleranceFlag = "-t";
static const char* toleranceFlagLong = "-tolerance";

MSyntax TransferVertex::newSyntax()
{
	MSyntax syntax;

	MStatus status;
	status = syntax.addFlag(sourceUvSetFlag, sourceUvSetFlagLong, MSyntax::kString);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	status = syntax.addFlag(targetUvSetFlag, targetUvSetFlagLong, MSyntax::kString);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	status = syntax.addFlag(sourceMeshFlag, sourceMeshFlagLong, MSyntax::kString);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	status = syntax.addFlag(targetMeshFlag, targetMeshFlagLong, MSyntax::kString);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	status = syntax.addFlag(toleranceFlag, toleranceFlagLong, MSyntax::kDouble);

	return syntax;
}

TransferVertex::TransferVertex() {
}

TransferVertex::~TransferVertex() {
}

MStatus TransferVertex::doIt( const MArgList& args)
{
    MStatus status;
    // if (args.length() != 1) {
    //     MGlobal::displayError("Need one arg");
    //     return MStatus::kFailure;
    // }

    // arg
    // MString argument = args.asString(0, &status);
    // if (status != MS::kSuccess) {
    //     return MStatus::kFailure;
    // }
    // CHECK_MSTATUS_AND_RETURN_IT(status);

	MArgDatabase argData(syntax(), args);

	if (argData.isFlagSet(sourceUvSetFlag)) {
		status = argData.getFlagArgument(sourceUvSetFlag, 0, sourceUvSet);
	}
	else {
		sourceUvSet = "map1";
	}

	if (argData.isFlagSet(targetUvSetFlag)) {
		status = argData.getFlagArgument(targetUvSetFlag, 0, targetUvSet);
	}
	else {
		targetUvSet = "map1";
	}

	if (argData.isFlagSet(toleranceFlag)) {
		status = argData.getFlagArgument(toleranceFlag, 0, tolerance);
	}
	else {
		tolerance = 0.0001;
	}

	if (argData.isFlagSet(sourceMeshFlag)) {
		status = argData.getFlagArgument(sourceMeshFlag, 0, sourceMesh);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}
	else {
		MGlobal::displayError("source mesh not specified");
		return MS::kFailure;
	}

	if (argData.isFlagSet(targetMeshFlag)) {
		status = argData.getFlagArgument(targetMeshFlag, 0, targetMesh);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}
	else {
		MGlobal::displayError("target mesh not specified");
		return MS::kFailure;
	}

    MSelectionList mList;
    // MGlobal::getActiveSelectionList(mList);

	mList.add(sourceMesh);
	mList.add(targetMesh);

    MDagPath sourceDagPath;
    MDagPath targetDagPath;

    mList.getDagPath(0, sourceDagPath);
    mList.getDagPath(1, targetDagPath);

    MGlobal::displayInfo(sourceDagPath.fullPathName());
    MGlobal::displayInfo(targetDagPath.fullPathName());

    sourceFnMesh.setObject(sourceDagPath);
    targetFnMesh.setObject(targetDagPath);

    MString uvSetName = "map1";
    MString* uvSetPtr = &uvSetName;

    MIntArray polygonIds;
    MPointArray points;

    MIntArray faceIndices;
    targetFnMesh.getPoints(originalPositions);
    targetFnMesh.getPoints(newPositions);


	// UVset
	MString souceUVSetOrig = sourceFnMesh.currentUVSetName();
	sourceFnMesh.setCurrentUVSetName(sourceUvSet);
	MString targetUVSetOrig = targetFnMesh.currentUVSetName();
	targetFnMesh.setCurrentUVSetName(targetUvSet);

    for (MItMeshVertex itVerts(targetDagPath); !itVerts.isDone(); itVerts.next()) {
        itVerts.getConnectedFaces(faceIndices);

        int numFaces = faceIndices.length();
        for (int i=0; i<numFaces; i++) {
            int index = faceIndices[i];
            float uv[2];
            itVerts.getUV(index, uv, &targetUvSet);

            status = sourceFnMesh.getPointsAtUV(
                polygonIds,
                points,
                uv,
                MSpace::kObject,
                &sourceUvSet,
                tolerance);

            if (status == MS::kSuccess) {
				int length = points.length();
				if (length != 0) {
					newPositions[itVerts.index()] = points[0];
				}
			}
        }
    }

    targetFnMesh.setPoints(newPositions);

	sourceFnMesh.setCurrentUVSetName(souceUVSetOrig);
	targetFnMesh.setCurrentUVSetName(targetUVSetOrig);

    return redoIt();
}

MStatus TransferVertex::redoIt() {
    MStatus status;
    return status;
}


MStatus TransferVertex::undoIt() {
    targetFnMesh.setPoints(originalPositions);
    return MS::kSuccess;
}


bool TransferVertex::isUndoable() const {
    return true;
}

void* TransferVertex::creator()
{
    return new TransferVertex;
}
