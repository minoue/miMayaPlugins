#ifndef __FINDUVOVERLAPS2_H__
#define __FINDUVOVERLAPS2_H__

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MString.h>

#include <set>
#include <unordered_set>
#include "uvEdge.h"
#include "uvShell.h"


class FindUvOverlaps2 : public MPxCommand {
public:
    FindUvOverlaps2();
    virtual ~FindUvOverlaps2();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creator();
    static MSyntax newSyntax();
    MStatus check(std::unordered_set<UvEdge, hash_edge>& edges, std::unordered_set<int>& result);
    bool isShellOverlapped(UvShell& shellA, UvShell& shellB);
    void makeCombinations(size_t N, std::vector<std::vector<int> >& vec);

private:
    bool verbose;
    MDagPath mDagPath;
    MFnMesh mFnMesh;
    MString uvSet;
};

#endif /* defined(__FINDUVOVERLAPS2_H__) */
