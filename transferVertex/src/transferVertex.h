// transferVertex.h
//
// Copyright (c) 2019 Michitaka Inoue
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#ifndef __TransferVertex_H__
#define __TransferVertex_H__

#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>
#include <maya/MPointArray.h>
#include <maya/MFnMesh.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>


class TransferVertex : public MPxCommand
{
public:
    TransferVertex();
    virtual ~TransferVertex();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creator();
	static MSyntax newSyntax();

private:
    MDagPath mDagPath;
    MPointArray originalPositions;
    MPointArray newPositions;
    MFnMesh sourceFnMesh;
    MFnMesh targetFnMesh;

	MString sourceUvSet;
	MString targetUvSet;
	MString sourceMesh;
	MString targetMesh;

	double tolerance;
};


#endif /* defined(__TransferVertex_H__) */
