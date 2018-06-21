//
//  snapToClosest.h
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


#ifndef __snapCmd__snapToClosest__
#define __snapCmd__snapToClosest__


#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MSelectionList.h>
#include <maya/MSyntax.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>


class SnapToClosest : public MPxCommand
{
public:
    SnapToClosest();
    virtual ~SnapToClosest();
    virtual MStatus doIt(const MArgList& argList);
    virtual MStatus undoIt();
    virtual MStatus redoIt();
    bool isUndoable() const;
    static void* creator();
    static MSyntax newSyntax();

private:
    MDagPath            mDagPath_components;
    MDagPath            mDagPath_target;
    MObject             components;
    MFnMesh             fnMeshComponents;
    MFnMesh             fnMeshTarget;
    MPointArray         vertexArray;
    MPointArray         newVertexArray;  
    MSelectionList      mList;             
    MString             targetObjectName;         
    bool                dummyBool;
    double              searchDistance;
    MString             mode;
    bool                useCustomVector;
    MVector             customVector;
    double              customVectorX;
    double              customVectorY;
    double              customVectorZ;
};

#endif /* defined(__snapCmd__snapToClosest__) */
