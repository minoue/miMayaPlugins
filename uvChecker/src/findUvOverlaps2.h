#ifndef __FINDUVOVERLAPS2_H__
#define __FINDUVOVERLAPS2_H__

#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

#include "uvEdge.h"
#include "uvShell.h"
#include "event.h"
#include <set>
#include <unordered_set>
#include <deque>

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
    MStatus check(std::set<UvEdge>& edges);
    MStatus checkEdgesAndCreateEvent(UvEdge& edgeA, UvEdge& edgeB, std::deque<Event>& eventQueue);
    bool isShellOverlapped(UvShell& shellA, UvShell& shellB);
    void makeCombinations(size_t N, std::vector<std::vector<int>>& vec);
    bool doBegin(Event& currentEvent, std::deque<Event>& eventQueue, std::vector<UvEdge>& statusQueue);
    bool doEnd(Event& currentEvent, std::deque<Event>& eventQueue, std::vector<UvEdge>& statusQueue);
    bool doCross(Event& currentEvent, std::deque<Event>& eventQueue, std::vector<UvEdge>& statusQueue);

private:
    bool verbose;
    MDagPath mDagPath;
    MFnMesh mFnMesh;
    MString uvSet;
    
    // u and v values of crossing point of two edges
    float intersect_u;
    float intersect_v;
    
    // Countainer for a set of UV indices for the final result
    std::unordered_set<int> resultSet;

};

#endif /* defined(__FINDUVOVERLAPS2_H__) */
