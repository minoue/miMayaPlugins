#include "findUvOverlaps.h"
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MStringArray.h>
#include <maya/MTimer.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <unordered_map>
#include <string>

#define NUM_TASKS 16

MDagPath FindUvOverlaps::mDagPath;

typedef struct _taskDataTag {
    int start;
    int end;
    MString fullPath;
    MIntArray innerIntersections;
    int* boolArray;
} taskData;

typedef struct _threadDataTag {
    int threadNo;
    int start;
    int end;
    MString name;
    taskData* tdata;
} threadData;

FindUvOverlaps::FindUvOverlaps()
{
}

FindUvOverlaps::~FindUvOverlaps()
{
}

MSyntax FindUvOverlaps::newSyntax()
{
    MSyntax syntax;
    syntax.addArg(MSyntax::kString);
    syntax.addFlag("-v", "-verbose", MSyntax::kBoolean);
    return syntax;
}

void combination(int N, std::vector<std::vector<int> >& vec)
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

float FindUvOverlaps::getTriangleArea(float& Ax, float& Ay, float& Bx, float& By, float& Cx, float& Cy) {
    float area = ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) / 2.0;
    return area;
}

bool FindUvOverlaps::checkShellIntersection(UVShell& s1, UVShell& s2)
{
    bool uIntersection = true;
    bool vIntersection = true;

    if (s1.uMax < s2.uMin || s1.uMin > s2.uMax) {
        uIntersection = false;
    }
    if (s1.vMax < s2.vMin || s1.vMin > s2.vMax) {
        vIntersection = false;
    }
    if (uIntersection == false || vIntersection == false) {
        return false;
    } else {
        return true;
    }
}

bool FindUvOverlaps::checkCrossingNumber(float& u, float& v, std::vector<int>& uvIds)
{
    float u_current;
    float v_current;
    float u_next;
    float v_next;
    float u2 = u+10.0;
    float area1;
    float area2;

    int polygonVertexCount = uvIds.size();
    int lastIndex = polygonVertexCount - 1;

    int numIntersections = 0;
    for (int currentIndex=0; currentIndex<polygonVertexCount; currentIndex++) {
        bool toggleA = true;
        bool toggleB = true;
        if (currentIndex == lastIndex) {
            int& currentID = uvIds[currentIndex];
            int& nextID = uvIds[0];
            u_current = uArray[currentID];
            v_current = vArray[currentID];
            u_next = uArray[nextID];
            v_next = vArray[nextID];
        }
        else {
            int& currentID = uvIds[currentIndex];
            int& nextID = uvIds[currentIndex+1];
            u_current = uArray[currentID];
            v_current = vArray[currentID];
            u_next = uArray[nextID];
            v_next = vArray[nextID];
        }
        area1 = getTriangleArea(u, v, u_current, v_current, u2, v);
        area2 = getTriangleArea(u, v, u_next, v_next, u2, v);
        if ((area1 > 0.0 && area2 > 0.0) || (area1 < 0.0 && area2 < 0.0)) {
            toggleA = false;
        }
        area1 = getTriangleArea(u_current, v_current, u, v, u_next, v_next);
        area2 = getTriangleArea(u_current, v_current, u2, v, u_next, v_next);
        if ((area1 > 0.0 && area2 > 0.0) || (area1 < 0.0 && area2 < 0.0)) {
            toggleB = false;
        }
        if (toggleA == true && toggleB == true) {
            numIntersections++;
        }
    }

    if ((numIntersections % 2) != 0) {
        return true;
    } else {
        return false;
    }
}

MStatus FindUvOverlaps::findShellIntersections(UVShell& shellA, UVShell& shellB)
{
    MStatus status;

    int numUVsInShellA = shellA.uvPoints.size();

    int numBorderPoints = shellA.borderUvPoints.size();

    float u;
    float v;
    float u2 = u+10.0;

    float area1;
    float area2;

    MIntArray uvCounts;
    MIntArray uvIds;

    fnMesh.getAssignedUVs(uvCounts, uvIds);

    std::unordered_map<int, std::vector<int> > uvMap;

    int counter = 0;
    for (int i=0; i<uvCounts.length(); i++) {
        int count = uvCounts[i];
        std::vector<int> uvs;
        for (int c=0; c<count; c++) {
            uvs.push_back(uvIds[counter]);
            counter++;
        }
        uvMap[i] = uvs;
    }

    for (int s = 0; s < numBorderPoints; s++) {
        fnMesh.getUV(shellA.borderUvPoints[s], u, v);

        if (u < shellB.uMin || u > shellB.uMax) {
            continue;
        }
        if (v < shellB.vMin || v > shellB.vMax) {
            continue;
        }

        std::unordered_set<int>::iterator polygonIter;
        int polygonFaceId;

        for (polygonIter = shellB.polygonIDs.begin(); polygonIter != shellB.polygonIDs.end(); ++polygonIter) {
            polygonFaceId = *polygonIter;
            std::vector<int>& polygonUvIds = uvMap[polygonFaceId];

            bool isInPolygon = checkCrossingNumber(u, v, polygonUvIds);
            if (isInPolygon == true) {
                shellIntersectionsResult.append(polygonFaceId);
                break;
            }
        }
    }
    return MS::kSuccess;
}

MStatus FindUvOverlaps::createTaskData(int numPolygons, MString name)
{
    MStatus stat = MThreadPool::init();
    if (MStatus::kSuccess != stat) {
        MString str = MString("Error creating threadpool");
        MGlobal::displayError(str);
        return MS::kFailure;
    }

    // int numPolygons = end + 1;

    taskData tdata;
    tdata.start = 0;
    tdata.end = numPolygons - 1;
    tdata.fullPath = name;
    tdata.boolArray = new int[numPolygons]();
    MThreadPool::newParallelRegion(createThreadData, (void*)&tdata);

    // pool is reference counted. Release reference to current thread instance
    MThreadPool::release();

    // release reference to whole pool which deletes all threads
    // MThreadPool::release();
    delete[] tdata.boolArray;

    // std::cout << "size of array" << tdata.resultIndexArray.length() << std::endl;
    if (tdata.innerIntersections.length() != 0) {
        for (int i = 0; i < tdata.innerIntersections.length(); i++) {
            innerIntersectionsResult.copy(tdata.innerIntersections);
        }
    }

    return MS::kSuccess;
}

void FindUvOverlaps::createThreadData(void* data, MThreadRootTask* root)
{
    taskData* taskD = (taskData*)data;

    threadData tdata[NUM_TASKS];

    int numFaces = taskD->end + 1;
    int taskLength = (numFaces + NUM_TASKS - 1) / NUM_TASKS;
    int start = 0;
    int end = taskLength;
    int lastTask = NUM_TASKS - 1;

    for (int i = 0; i < NUM_TASKS; ++i) {
        if (i == lastTask) {
            end = numFaces;
        }
        tdata[i].threadNo = i;
        tdata[i].start = start;
        tdata[i].end = end;
        tdata[i].name = taskD->fullPath;
        tdata[i].tdata = taskD;

        start += taskLength;
        end += taskLength;

        MThreadPool::createTask(findInnerIntersectionsMT, (void*)&tdata[i], root);
    }

    MThreadPool::executeAndJoin(root);

    for (int i = 0; i < numFaces; i++) {
        if (taskD->boolArray[i] == 1) {
            taskD->innerIntersections.append(i);
        }
    }
}

MThreadRetVal FindUvOverlaps::findInnerIntersectionsMT(void* data)
{
    threadData* myData = (threadData*)data;

    MFnMesh fnMesh(mDagPath);

    int vertexList[3];
    MIntArray vertexIdArray;
    std::map<int, int> localVtxIdMap;

    for (int faceId = myData->start; faceId < myData->end; faceId++) {
        fnMesh.getPolygonVertices(faceId, vertexIdArray);
        int numTriangles = vertexIdArray.length() - 2;

        for (int localId = 0; localId < vertexIdArray.length(); localId++) {
            localVtxIdMap[vertexIdArray[localId]] = localId;
        }

        for (int triId = 0; triId < numTriangles; triId++) {
            UVPoint uvPointArray[3];
            fnMesh.getPolygonTriangleVertices(faceId, triId, vertexList);
            float u;
            float v;
            for (int vtx = 0; vtx < 3; vtx++) {
                int localIndex = localVtxIdMap[vertexList[vtx]];
                fnMesh.getPolygonUV(faceId, localIndex, u, v);
                uvPointArray[vtx].u = u;
                uvPointArray[vtx].v = v;
            }

            float& Ax = uvPointArray[0].u;
            float& Ay = uvPointArray[0].v;
            float& Bx = uvPointArray[1].u;
            float& By = uvPointArray[1].v;
            float& Cx = uvPointArray[2].u;
            float& Cy = uvPointArray[2].v;
            float area = ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) / 2;

            if (area < 0) {
                myData->tdata->boolArray[faceId] = 1;
            }
        }
    }
    return (MThreadRetVal)0;
}

MStatus FindUvOverlaps::doIt(const MArgList& args)
{
    MStatus status;

    MSelectionList sel;

    MArgDatabase argData(syntax(), args);

    status = argData.getCommandArgument(0, sel);
    if (status != MS::kSuccess) {
        MGlobal::displayError("You have to provide an object path");
        return MStatus::kFailure;
    }

    if (argData.isFlagSet("-verbose"))
        argData.getFlagArgument("-verbose", 0, verbose);
    else
        verbose = false;

    sel.getDagPath(0, mDagPath);

    if (verbose == true) {
        MString objectPath = "Selected mesh : " + mDagPath.fullPathName();
        MGlobal::displayInfo(objectPath);
    }

    status = mDagPath.extendToShape();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (mDagPath.apiType() != MFn::kMesh) {
        MGlobal::displayError("Selected object is not mesh.");
        return MStatus::kFailure;
    }

    return redoIt();
}

MStatus FindUvOverlaps::redoIt()
{
    MStatus status;

    // Setup timer
    MTimer timer;
    timer.beginTimer();

    // Get basic mesh information
    fnMesh.setObject(mDagPath);
    int numFaces = fnMesh.numPolygons();
    int numUVs = fnMesh.numUVs();
    int numVerts = fnMesh.numVertices();
    MString fullPath = mDagPath.fullPathName();

    // run multithread
    status = createTaskData(numFaces, fullPath);

    MIntArray uvShellIds;
    unsigned int numUVshells;
    fnMesh.getUvShellsIds(uvShellIds, numUVshells);
    if (numUVshells == 1) {
        // Do nothing when there is only one UV shell
    } else {
        // Setup uv shell objects
        std::vector<UVShell> uvShellArray;
        uvShellArray.resize(numUVshells);
        for (int i = 0; i < numUVshells; i++) {
            UVShell shell;
            uvShellArray[i] = shell;
        }

        // Add polygonIDs to each UV shell
        MIntArray uvIndexArray;
        MIntArray connectedFacesArray;
        int numUniUv;
        for (MItMeshVertex itVerts(mDagPath); !itVerts.isDone(); itVerts.next()) {
            itVerts.numUVs(numUniUv);
            itVerts.getUVIndices(uvIndexArray);
            if (numUniUv == 1) {
                // If current vertex has only 1 UV point, its UV is inside of a UV shell
                int thisIndex = uvIndexArray[0];
                int shellNumber = uvShellIds[thisIndex];
                itVerts.getConnectedFaces(connectedFacesArray);
                for (int f = 0; f < connectedFacesArray.length(); f++) {
                    int connectedFaceID = connectedFacesArray[f];
                    uvShellArray[shellNumber].polygonIDs.insert(connectedFaceID);
                }
            } else {
                // If current vertex has multiple UV points, its UVs are on a shell border
                for (int uvi = 0; uvi < uvIndexArray.length(); uvi++) {
                    int uvIndex = uvIndexArray[uvi];
                    int shellNumber = uvShellIds[uvIndex];
                    uvShellArray[shellNumber].borderUvPoints.push_back(uvIndex);
                }
            }
        }

        // commnets here
        fnMesh.getUVs(uArray, vArray);

        for (int i = 0; i < numUVs; i++) {
            UVPoint p;
            p.u = uArray[i];
            p.v = vArray[i];
            p.index = i;
            p.shellIndex = uvShellIds[i];
            uvShellArray[uvShellIds[i]].uvPoints.push_back(p);
            uvShellArray[uvShellIds[i]].uVector.push_back(uArray[i]);
            uvShellArray[uvShellIds[i]].vVector.push_back(vArray[i]);
        }

        // Get min and max for each bounding box
        for (int i = 0; i < numUVshells; i++) {
            UVShell& shell = uvShellArray[i];
            shell.uMax = *std::max_element(shell.uVector.begin(), shell.uVector.end());
            shell.uMin = *std::min_element(shell.uVector.begin(), shell.uVector.end());
            shell.vMax = *std::max_element(shell.vVector.begin(), shell.vVector.end());
            shell.vMin = *std::min_element(shell.vVector.begin(), shell.vVector.end());
        }

        // comments here
        std::vector<std::vector<int> > shellCombVec;
        combination(numUVshells, shellCombVec);

        for (int i = 0; i < shellCombVec.size(); i++) {
            int& shellIndexA = shellCombVec[i][0];
            int& shellIndexB = shellCombVec[i][1];
            UVShell& shellA = uvShellArray[shellIndexA];
            UVShell& shellB = uvShellArray[shellIndexB];

            bool isIntersected = checkShellIntersection(shellA, shellB);

            if (isIntersected == true) {
                findShellIntersections(shellA, shellB);
                findShellIntersections(shellB, shellA);
            } else {
            }
        }
    }

    // setup and return result
    MStringArray resultStrArray;
    for (int i = 0; i < innerIntersectionsResult.length(); i++) {
        MString index;
        index.set(innerIntersectionsResult[i]);
        MString n = fullPath + ".f[" + index + "]";
        resultStrArray.append(n);
    }
    for (int i = 0; i < shellIntersectionsResult.length(); i++) {
        MString index;
        index.set(shellIntersectionsResult[i]);
        MString n = fullPath + ".f[" + index + "]";
        resultStrArray.append(n);
    }
    MPxCommand::setResult(resultStrArray);

    // Show time
    timer.endTimer();
    double resultTime = timer.elapsedTime();
    MString timeStr;
    timeStr.set(resultTime);
    MString r = "Result : " + timeStr + " seconds.";
    MGlobal::displayInfo(r);

    return MS::kSuccess;
}

MStatus FindUvOverlaps::undoIt()
{
    return MS::kSuccess;
}

bool FindUvOverlaps::isUndoable() const
{
    return false;
}

void* FindUvOverlaps::creater()
{
    return new FindUvOverlaps;
}
