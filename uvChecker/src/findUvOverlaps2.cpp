#include "findUvOverlaps2.h"
#include "uvShell.h"
#include "uvPoint.h"
#include "uvEdge.h"

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MIntArray.h>
#include <maya/MString.h>

#include <vector>
#include <algorithm>
#include <utility>


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
    
    status = mDagPath.extendToShape();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (mDagPath.apiType() != MFn::kMesh) {
        MGlobal::displayError("Selected object is not mesh.");
        return MStatus::kFailure;
    }

    if (verbose) 
        MGlobal::displayInfo("Target object : " + mDagPath.fullPathName());

    return redoIt();
}

MStatus FindUvOverlaps2::redoIt()
{
    MStatus status;

    MIntArray uvShellIds;
    unsigned int nbUvShells;
    mFnMesh.getUvShellsIds(uvShellIds, nbUvShells);
    int numPolygons = mFnMesh.numPolygons();

    // Setup uv shell objects
    std::vector<UvShell> uvShellArray;
    uvShellArray.resize(nbUvShells);
    for (unsigned int i = 0; i < nbUvShells; i++) {
        UvShell shell;
        shell.shellIndex = i;
        uvShellArray[i] = shell;
    }

    for (unsigned int uvId = 0; uvId < uvShellIds.length(); uvId++) {
        float u, v;
        mFnMesh.getUV(uvId, u, v);
        UvShell& currentShell = uvShellArray[uvShellIds[uvId]];
        currentShell.uVector.push_back(u);
        currentShell.vVector.push_back(v);
    }

    for (unsigned int id = 0; id < nbUvShells; id++) {
        UvShell& shell = uvShellArray[id];
        int size = shell.uVector.size();
        float uMax = *std::max_element(shell.uVector.begin(), shell.uVector.end());
        float vMax = *std::max_element(shell.vVector.begin(), shell.vVector.end());
        float uMin = *std::min_element(shell.uVector.begin(), shell.uVector.end());
        float vMin = *std::min_element(shell.vVector.begin(), shell.vVector.end());
        shell.uMax = uMax;
        shell.vMax = vMax;
        shell.uMin = uMin;
        shell.vMin = vMin;
    }

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

            // UV indecis by local order
            int uvIdA;
            int uvIdB;
            mFnMesh.getPolygonUVid(faceId, curLocalIndex, uvIdA);
            mFnMesh.getPolygonUVid(faceId, nextLocalIndex, uvIdB);
            int currentShellIndex = uvShellIds[uvIdA];

            std::pair<int, int> edgeIndex;
            if (uvIdA < uvIdB)
                edgeIndex = std::make_pair(uvIdA, uvIdB);
            else
                edgeIndex = std::make_pair(uvIdB, uvIdA);

            // Get UV values and create edge objects
            float u_current, v_current;
            float u_next, v_next;
            mFnMesh.getPolygonUV(faceId, curLocalIndex, u_current, v_current);
            mFnMesh.getPolygonUV(faceId, nextLocalIndex, u_next, v_next);
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

            UvEdge edge(beginPt, endPt, edgeIndex);
            uvShellArray[currentShellIndex].edgeSet.insert(edge);
        }
    }

    // UvShell& testShell = uvShellArray[0];
    // int numEdges = testShell.edgeSet.size();
    // MString test;
    // test.set(numEdges);
    // MGlobal::displayInfo(test);
    
    return MS::kSuccess;
}

MStatus FindUvOverlaps2::undoIt()
{
    return MS::kSuccess;
}

bool FindUvOverlaps2::isUndoable() const
{
    return false;
}

void* FindUvOverlaps2::creater()
{
    return new FindUvOverlaps2;
}
