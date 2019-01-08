// transferUV.cpp
//
// Copyright (c) 2018 Michitaka Inoue
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php


#include "transferUV.h"
#include <iostream>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

// Flags for this command
static const char* sourceUvSetFlag = "-suv";
static const char* sourceUvSetFlagLong = "-sourceUvSet";
static const char* targetUvSetFlag = "-tuv";
static const char* targetUvSetFlagLong = "-targetUvSet";
static const char* sourceMeshFlag = "-sm";
static const char* sourceMeshFlagLong = "-sourceMesh";
static const char* targetMeshFlag = "-tm";
static const char* targetMeshFlagLong = "-targetMesh";

TransferUV::TransferUV()
{
}

TransferUV::~TransferUV()
{
}

MSyntax TransferUV::newSyntax()
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

    return syntax;
}

MStatus TransferUV::doIt(const MArgList& args)
{
    MStatus status;

    // if (args.length() != 1)
    // {
    //     MGlobal::displayError("Need 1 arg!");
    //     return MStatus::kFailure;
    // }

    MArgDatabase argData(syntax(), args);

    // arg
    // MString argument = args.asString(0, &status);
    // if (status != MS::kSuccess) {
    //     return MStatus::kFailure;
    // }
    // CHECK_MSTATUS_AND_RETURN_IT(status);

    if (argData.isFlagSet(sourceUvSetFlag)) {
        status = argData.getFlagArgument(sourceUvSetFlag, 0, sourceUvSet);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    if (argData.isFlagSet(targetUvSetFlag)) {
        status = argData.getFlagArgument(targetUvSetFlag, 0, targetUvSet);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    if (argData.isFlagSet(sourceMeshFlag)) {
        status = argData.getFlagArgument(sourceMeshFlag, 0, sourceMesh);
    } else {
        MGlobal::displayError(
            "Source mesh is not specified. -sm flag is required.");
        return MS::kFailure;
    }

    if (argData.isFlagSet(targetMeshFlag)) {
        status = argData.getFlagArgument(targetMeshFlag, 0, targetMesh);
    } else {
        MGlobal::displayError(
            "Target mesh is not specified. -tm flag is required.");
        return MS::kFailure;
    }

    MString info = "Copying uv from " + sourceUvSet + " to " + targetUvSet;
    MGlobal::displayInfo(info);

    MSelectionList mList;
    mList.add(sourceMesh);
    mList.add(targetMesh);

    mList.getDagPath(0, sourceDagPath);
    mList.getDagPath(1, targetDagPath);

    return redoIt();
}

MStatus TransferUV::redoIt()
{
    MStatus status;

    MFnMesh sourceFnMesh(sourceDagPath);
    MFnMesh targetFnMesh(targetDagPath);
    
    // Get current UV sets and keep them to switch back to them at the last
    MString currentSourceUVSet = sourceFnMesh.currentUVSetName();
    MString currentTargetUVSet = targetFnMesh.currentUVSetName();

    // Switch to uv sets specified in flags before starting transfer process
    // This is required because MFnMesh does not work properly with
    // "Non-current" UV sets
    sourceFnMesh.setCurrentUVSetName(sourceUvSet);
    targetFnMesh.setCurrentUVSetName(targetUvSet);
    
    MString* sourceUvSetPtr = &sourceUvSet;
    MString* targetUvSetPtr = &targetUvSet;

    MFloatArray sourceUarray;
    MFloatArray sourceVarray;
    MIntArray sourceUvCounts;
    MIntArray sourceUvIds;

    // Get mesh information for each
    status = sourceFnMesh.getUVs(sourceUarray, sourceVarray, sourceUvSetPtr);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to get source UVs");
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    status = targetFnMesh.getUVs(originalUarray, originalVarray, targetUvSetPtr);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to get original UVs");
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    status = sourceFnMesh.getAssignedUVs(sourceUvCounts, sourceUvIds, sourceUvSetPtr);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to get source assigned UVs");
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    status = targetFnMesh.getAssignedUVs(originalUvCounts, originalUvIds, targetUvSetPtr);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to get original assigned UVs");
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // Resize source uv array so it becomes same size as number of targets uv
    unsigned int targetNumUVs = targetFnMesh.numUVs();
    if (targetNumUVs > sourceUarray.length()) {
        sourceUarray.setLength(targetNumUVs);
        sourceVarray.setLength(targetNumUVs);
    }

    status = targetFnMesh.setUVs(sourceUarray, sourceVarray, targetUvSetPtr);
    if (MS::kSuccess != status) {
        MGlobal::displayError("Failed to set source UVs");
        return status;
    }

    status = targetFnMesh.assignUVs(sourceUvCounts, sourceUvIds, targetUvSetPtr);
    if (MS::kSuccess != status) {
        MGlobal::displayError("Failed to assign source UVs");
        return status;
    }

    // Switch back to originals
    sourceFnMesh.setCurrentUVSetName(currentSourceUVSet);
    targetFnMesh.setCurrentUVSetName(currentTargetUVSet);
    
    return MS::kSuccess;
}

MStatus TransferUV::undoIt()
{
    MStatus status;

    MFnMesh undoMesh(targetDagPath);
    MGlobal::displayInfo(targetDagPath.fullPathName());

    MString* targetUvSetPtr = &targetUvSet;

    // Resize original uv array so it becomes same size as current num uvs
    unsigned int numUVs = undoMesh.numUVs();
    if (numUVs > originalUarray.length()) {
        originalUarray.setLength(undoMesh.numUVs());
        originalVarray.setLength(undoMesh.numUVs());
    }

    status = undoMesh.setUVs(originalUarray, originalVarray, targetUvSetPtr);
    if (MS::kSuccess != status) {
        MGlobal::displayError("Failed to set original UVs");
        return status;
    } else {
        MGlobal::displayError("OK");
    }

    status = undoMesh.assignUVs(originalUvCounts, originalUvIds, targetUvSetPtr);
    if (MS::kSuccess != status) {
        MGlobal::displayError("Failed to assgin original UVs");
        return status;
    }

    return MS::kSuccess;
}

bool TransferUV::isUndoable() const
{
    return true;
}

void* TransferUV::creator()
{
    return new TransferUV;
}
