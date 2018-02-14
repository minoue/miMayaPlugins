#include "findUvOverlaps2.h"
#include "uvPoint.h"
#include "event.h"
#include "testCase.h"

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MIntArray.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <deque>
#include <iterator>


FindUvOverlaps2::FindUvOverlaps2()
{
}

FindUvOverlaps2::~FindUvOverlaps2()
{
}

MSyntax FindUvOverlaps2::newSyntax()
{
    MSyntax syntax;
    syntax.addArg(MSyntax::kString);
    syntax.addFlag("-v", "-verbose", MSyntax::kBoolean);
    syntax.addFlag("-set", "-uvSet", MSyntax::kString);
    return syntax;
}


MStatus FindUvOverlaps2::doIt(const MArgList& args)
{
    MStatus status;

    MSelectionList sel;

    MArgDatabase argData(syntax(), args);

    status = argData.getCommandArgument(0, sel);
    if (status != MS::kSuccess) {
        MGlobal::displayError("You have to provide an object path");
        return MStatus::kFailure;
    }

    sel.getDagPath(0, mDagPath);
    mFnMesh.setObject(mDagPath);

    if (argData.isFlagSet("-verbose"))
        argData.getFlagArgument("-verbose", 0, verbose);
    else
        verbose = false;

    if (argData.isFlagSet("-uvSet"))
        argData.getFlagArgument("-uvSet", 0, uvSet);
    else
        uvSet = "map1";

    // Check if specified object is geometry or not
    status = mDagPath.extendToShape();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Check if specified uv set exists
    MStringArray uvSetNames;
    bool uvSetFound;
    mFnMesh.getUVSetNames(uvSetNames);
    for (int uv=0; uv<uvSetNames.length(); uv++) {
        MString& uvSetName = uvSetNames[uv];
        if (uvSetName == uvSet)
            uvSetFound = true;
        else
            uvSetFound = false;
    }

    if (uvSetFound == false) {
        MGlobal::displayError("Error: uv set not found\n");
        return MS::kFailure;
    }

    if (mDagPath.apiType() != MFn::kMesh) {
        MGlobal::displayError("Selected object is not mesh.");
        return MStatus::kFailure;
    }

    if (verbose)
        MGlobal::displayInfo("Target object : " + mDagPath.fullPathName());
        MGlobal::displayInfo("UVset for check : " + uvSet);

    return redoIt();
}

MStatus FindUvOverlaps2::redoIt()
{
    MStatus status;

    const MString* uvSetPtr = &uvSet;

    MIntArray uvShellIds;
    unsigned int nbUvShells;
    mFnMesh.getUvShellsIds(uvShellIds, nbUvShells, uvSetPtr);

    // if no UVs are detected on this mesh
    if (nbUvShells == 0) {
        MGlobal::displayError("No UVs are found.");
        return MS::kFailure;
    }

    int numUVs = mFnMesh.numUVs(uvSet);
    int numPolygons = mFnMesh.numPolygons();

    // Setup uv shell objects
    std::vector<UvShell> uvShellArray;
    uvShellArray.resize(nbUvShells);
    for (unsigned int i = 0; i < nbUvShells; i++) {
        UvShell shell;
        shell.shellIndex = i;
        uvShellArray[i] = shell;
    }

    // Get UV values and add them to the shell
    for (unsigned int uvId = 0; uvId < numUVs; uvId++) {
        float u, v;
        mFnMesh.getUV(uvId, u, v, uvSetPtr);
        UvShell& currentShell = uvShellArray[uvShellIds[uvId]];
        currentShell.uVector.push_back(u);
        currentShell.vVector.push_back(v);
    }

    // Setup bounding box information for each shell
    for (unsigned int id = 0; id < nbUvShells; id++) {
        UvShell& shell = uvShellArray[id];
        float uMax = *std::max_element(shell.uVector.begin(), shell.uVector.end());
        float vMax = *std::max_element(shell.vVector.begin(), shell.vVector.end());
        float uMin = *std::min_element(shell.uVector.begin(), shell.uVector.end());
        float vMin = *std::min_element(shell.vVector.begin(), shell.vVector.end());
        shell.uMax = uMax;
        shell.vMax = vMax;
        shell.uMin = uMin;
        shell.vMin = vMin;
    }

    // Loop all polygon faces and create edge objects
    for (unsigned int faceId = 0; faceId < numPolygons; faceId++) {
        int numPolygonVertices = mFnMesh.polygonVertexCount(faceId);
        for (int localVtx=0; localVtx<numPolygonVertices; localVtx++) {
            int curLocalIndex;
            int nextLocalIndex;
            if (localVtx == numPolygonVertices-1) {
                curLocalIndex = localVtx;
                nextLocalIndex = 0;
            }
            else {
                curLocalIndex = localVtx;
                nextLocalIndex = localVtx+1;
            }

            // UV indices by local order
            int uvIdA;
            int uvIdB;

            // Check if current polygon face has mapped UVs, if not break this loop and go to next face
            MStatus statusA;
            MStatus statusB;
            statusA = mFnMesh.getPolygonUVid(faceId, curLocalIndex, uvIdA, uvSetPtr);
            statusB = mFnMesh.getPolygonUVid(faceId, nextLocalIndex, uvIdB, uvSetPtr);
            if (statusA != MS::kSuccess || statusB != MS::kSuccess) {
                if (verbose)
                    MGlobal::displayWarning("Non mapped faces are found");
                break;
            }

            int currentShellIndex = uvShellIds[uvIdA];

            std::pair<int, int> edgeIndex;
            if (uvIdA < uvIdB)
                edgeIndex = std::make_pair(uvIdA, uvIdB);
            else
                edgeIndex = std::make_pair(uvIdB, uvIdA);

            // Get UV values and create edge objects
            float u_current, v_current;
            float u_next, v_next;
            mFnMesh.getPolygonUV(faceId, curLocalIndex, u_current, v_current, uvSetPtr);
            mFnMesh.getPolygonUV(faceId, nextLocalIndex, u_next, v_next, uvSetPtr);
            UvPoint p1(u_current, v_current, uvIdA, currentShellIndex);
            UvPoint p2(u_next, v_next, uvIdB, currentShellIndex);

            UvPoint beginPt;
            UvPoint endPt;

            if (p1 > p2) {
                beginPt = p2;
                endPt = p1;
            }
            else {
                beginPt = p1;
                endPt = p2;
            }

            // Create edge objects and insert them to shell edge set
            UvEdge edge(beginPt, endPt, edgeIndex);
            uvShellArray[currentShellIndex].edgeSet.insert(edge);
        }
    }

    // Countainer for a set of overlapped shell edges
    std::vector<std::unordered_set<UvEdge, hash_edge> > overlappedShells;

    // Countainer for a set of shell indices that doesn't have be checked as single shell
    std::set<int> dontCheck;

    int numShells = uvShellArray.size();

    // Get combinations of shell indices eg. (0, 1), (0, 2), (1, 2),,,
    std::vector<std::vector<int>> shellCombinations;
    makeCombinations(numShells, shellCombinations);

    for (int i=0; i<shellCombinations.size(); i++) {
        UvShell& shellA = uvShellArray[shellCombinations[i][0]];
        UvShell& shellB = uvShellArray[shellCombinations[i][1]];

        if (isShellOverlapped(shellA, shellB)) {
            // If two shells are overlapped, combine them into one single shell
            std::unordered_set<UvEdge, hash_edge> combinedEdges;
            combinedEdges.insert(shellA.edgeSet.begin(), shellA.edgeSet.end());
            combinedEdges.insert(shellB.edgeSet.begin(), shellB.edgeSet.end());
            overlappedShells.push_back(combinedEdges);

            dontCheck.insert(shellA.shellIndex);
            dontCheck.insert(shellB.shellIndex);
        }
    }

    // Countainer for a set of UV indices for the final result
    std::unordered_set<int> resultSet;

    // Run checker for overlapped shells
    for (int s=0; s<overlappedShells.size(); s++) {
        check(overlappedShells[s], resultSet);
    }

    // Run checker for single shells
    for (int n=0; n<uvShellArray.size(); n++) {
        if (std::find(dontCheck.begin(), dontCheck.end(), n) != dontCheck.end()) {
            // if contains, do nothing
        }
        else {
			check(uvShellArray[n].edgeSet, resultSet);
        }
    }

    // Setup return result
    MStringArray resultStrArray;
    for (std::unordered_set<int>::iterator fsi = resultSet.begin(); fsi != resultSet.end(); ++fsi) {
        MString index;
        index.set(*fsi);
        MString fullPath = mDagPath.fullPathName();
        MString n = fullPath + ".map[" + index + "]";
        resultStrArray.append(n);
    }
    MPxCommand::setResult(resultStrArray);

    return MS::kSuccess;
}


bool FindUvOverlaps2::isShellOverlapped(UvShell& shellA, UvShell& shellB)
{
    if (shellA.uMax < shellB.uMin)
        return false;

    if (shellA.uMin > shellB.uMax)
        return false;

    if (shellA.vMax < shellB.vMin)
        return false;

    if (shellA.vMin > shellB.vMax)
        return false;

    return true;
}


MStatus FindUvOverlaps2::check(std::unordered_set<UvEdge, hash_edge>& edges, std::unordered_set<int>& resultSet)
{
    std::unordered_set<UvEdge, hash_edge>::iterator iter;

    std::deque<Event> eventQueue;

    int eventIndex = 0;
    for (iter = edges.begin(); iter != edges.end(); ++iter) {
        UvEdge edge = *iter;
        Event ev1("begin", edge.begin, edge, eventIndex);
        eventQueue.push_back(ev1);
        eventIndex += 1;
        Event ev2("end", edge.end, edge, eventIndex);
        eventQueue.push_back(ev2);
        eventIndex += 1;
    }

    std::sort(eventQueue.begin(), eventQueue.end());

    std::vector<UvEdge> statusQueue;
    statusQueue.reserve(edges.size());

    int numStatus;

    while(true) {
        if (eventQueue.empty()) {
            break;
        }
        Event firstEvent = eventQueue.front();
        UvEdge edge = firstEvent.edge;
        eventQueue.pop_front();

        if (firstEvent.status == "begin") {
            numStatus = statusQueue.size();
            statusQueue.push_back(edge);

            // Update x values of intersection to the sweepline for all edges
            // in the statusQueue
            for (int i = 0; i < statusQueue.size(); i++) {
                statusQueue[i].setCrossingPointX(firstEvent.v);
            }
            std::sort(statusQueue.begin(), statusQueue.end());

            auto foundIter = std::find(statusQueue.begin(), statusQueue.end(), edge);
            int index = std::distance(statusQueue.begin(), foundIter);
            if (index == statusQueue.size()) {
                // invalid
            }
            
            UvEdge& currentEdge = statusQueue[index];

            if (index == 0) {
                // If first item, check the next edge
                UvEdge& nextEdge = statusQueue[index+1];
                if (currentEdge.isIntersected(nextEdge)) {
                    resultSet.insert(currentEdge.index.first);
                    resultSet.insert(currentEdge.index.second);
                    resultSet.insert(nextEdge.index.first);
                    resultSet.insert(nextEdge.index.second);
                }
            }
            else if (index == statusQueue.size() - 1){
                UvEdge& previousEdge = statusQueue[index-1];
                if (currentEdge.isIntersected(previousEdge)) {
                    resultSet.insert(currentEdge.index.first);
                    resultSet.insert(currentEdge.index.second);
                    resultSet.insert(previousEdge.index.first);
                    resultSet.insert(previousEdge.index.second);
                }
            }
            else {
                UvEdge& nextEdge = statusQueue[index+1];
                UvEdge& previousEdge = statusQueue[index-1];
                
                if (currentEdge.isIntersected(nextEdge)) {
                    resultSet.insert(edge.index.first);
                    resultSet.insert(edge.index.second);
                    resultSet.insert(nextEdge.index.first);
                    resultSet.insert(nextEdge.index.second);
                }

                if (currentEdge.isIntersected(previousEdge)) {
                    resultSet.insert(edge.index.first);
                    resultSet.insert(edge.index.second);
                    resultSet.insert(previousEdge.index.first);
                    resultSet.insert(previousEdge.index.second);
                }
            }

            if (numStatus == 1) {
                continue;
            }

        }
        else if (firstEvent.status == "end") {
            numStatus = statusQueue.size();

            auto iter_for_removal = std::find(statusQueue.begin(), statusQueue.end(), edge);

            int removeIndex = std::distance(statusQueue.begin(), iter_for_removal);
            if (removeIndex == statusQueue.size()) {
                // invalid
            }

            if (numStatus <= 2) {
                // if num items are less than 2 in the countainer, do nothing
            }
            else if (removeIndex == 0) {
                // if first item, do nothing

            }
            else if (removeIndex == numStatus-1) {
                // if last item, do nothing
            }
            else {
                // check previous and next edge intersection as they can be next
                // each other after removing the current edge
                UvEdge& nextEdge = statusQueue[removeIndex + 1];
                UvEdge& previousEdge = statusQueue[removeIndex - 1];
                if (previousEdge.isIntersected(nextEdge)) {
                    resultSet.insert(nextEdge.index.first);
                    resultSet.insert(nextEdge.index.second);
                    resultSet.insert(previousEdge.index.first);
                    resultSet.insert(previousEdge.index.second);
                }
            }

            // Remove current edge from the statusQueue
            statusQueue.erase(iter_for_removal);
        }
        else {
            // cross
            // will be impremented later
        }
    }

    return MS::kSuccess;
}


/* https://stackoverflow.com/questions/12991758/creating-all-possible-k-combinations-of-n-items-in-c */
void FindUvOverlaps2::makeCombinations(int N, std::vector<std::vector<int>>& vec)
{
    std::string bitmask(2, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    // print integers and permute bitmask
    do {
        std::vector<int> sb;
        for (int i = 0; i < N; ++i) {
            if (bitmask[i]) {
                sb.push_back(i);
            }
        }
        vec.push_back(sb);
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
}


MStatus FindUvOverlaps2::undoIt()
{
    return MS::kSuccess;
}

bool FindUvOverlaps2::isUndoable() const
{
    return false;
}

void* FindUvOverlaps2::creator()
{
    return new FindUvOverlaps2;
}
