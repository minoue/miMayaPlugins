#include "sampleCommand.h"
#include <maya/MGlobal.h>
#include <maya/MArgList.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>


SampleCommand::SampleCommand() {
}

SampleCommand::~SampleCommand() {
}

MStatus SampleCommand::doIt( const MArgList& args)
{
    MStatus status;

    if (args.length() != 1) {
        MGlobal::displayError("Need one arg");
        return MStatus::kFailure;
    }

    // arg
    MString argument = args.asString(0, &status);
    if (status != MS::kSuccess) {
        return MStatus::kFailure;
    }
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Show arguments
    MString message = "Argument :" + argument;
    MGlobal::displayInfo(message);

    MSelectionList mList;
    MGlobal::getActiveSelectionList(mList);

    // Store the list of names
    MStringArray strArray;
    unsigned int length = mList.length();
    strArray.setLength(length);

    for (unsigned int i=0; i<length; i++) {
        mList.getDagPath(i, mDagPath);
        MString objPath = mDagPath.fullPathName();
        strArray.set(objPath, i);
    }

    MPxCommand::setResult(strArray);
    return redoIt();
}

MStatus SampleCommand::redoIt() {
    MStatus status;
    return status;
}

MStatus SampleCommand::undoIt() {
    return MS::kSuccess;
}

bool SampleCommand::isUndoable() const {
    return false;
}

void* SampleCommand::creator()

{
    return new SampleCommand;
}                                           
