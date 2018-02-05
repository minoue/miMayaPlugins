#ifndef __FINDUVOVERLAPS2_H__
#define __FINDUVOVERLAPS2_H__

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <set>
#include "uvEdge.h"


class FindUvOverlaps2 : public MPxCommand {
public:
    FindUvOverlaps2();
    virtual ~FindUvOverlaps2();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creater();
    static MSyntax newSyntax();
    MStatus check(std::set<UvEdge> edges, std::set<int>& result);

private:
    bool verbose;
    MDagPath mDagPath;
    MFnMesh mFnMesh;
};

#endif /* defined(__FINDUVOVERLAPS2_H__) */
