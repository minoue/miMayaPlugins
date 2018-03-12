#ifndef __FINDUVOVERLAPS2_H__
#define __FINDUVOVERLAPS2_H__

#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MSyntax.h>

#include "event.h"
#include "uvEdge.h"
#include "uvShell.h"

#include <deque>
#include <set>
#include <unordered_set>
#include <vector>

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

    MStatus check(const std::set<UvEdge>& edges);
    MStatus checkEdgesAndCreateEvent(UvEdge& edgeA, UvEdge& edgeB, std::deque<Event>& eventQueue);
    MStatus initializeObject(const MDagPath& dagPath, const int objectId);
    void makeCombinations(size_t N, std::vector<std::vector<int>>& vec);

    bool isShellOverlapped(UvShell& shellA, UvShell& shellB);
    bool doBegin(Event& currentEvent, std::deque<Event>& eventQueue, std::vector<UvEdge>& statusQueue);
    bool doEnd(Event& currentEvent, std::deque<Event>& eventQueue, std::vector<UvEdge>& statusQueue);
    bool doCross(Event& currentEvent, std::deque<Event>& eventQueue, std::vector<UvEdge>& statusQueue);

private:
    bool verbose;
    MDagPath dagPath;
    MString uvSet;
    MSelectionList mSel;

    // Container to store all UV shells to be tested
    std::vector<UvShell> uvShellArrayMaster;

    // u and v values of crossing point of two edges
    float intersect_u;
    float intersect_v;

    // Countainer for UVs of final result
    MStringArray resultStringArray;
};

#endif /* defined(__FINDUVOVERLAPS2_H__) */
