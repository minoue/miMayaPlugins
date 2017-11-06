#include "findUvOverlaps.h"
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MItMeshVertex.h>
#include <maya/MSelectionList.h>
#include <maya/MTimer.h>

#include <algorithm>
#include <iostream>
#include <string>

#define NUM_TASKS 16

MDagPath FindUvOverlaps::mDagPath;
MFloatArray FindUvOverlaps::uArray;
MFloatArray FindUvOverlaps::vArray;

struct taskDataTag {
    int start;
    int end;
    MIntArray innerIntersections;
    int* boolArray;
};

struct shellTaskDataTag {
    UvShell* shellA;
    UvShell* shellB;
    std::unordered_map<int, std::vector<int>>* uvMap;
    std::unordered_set<int> resultIndexSet;
};

struct threadDataTag {
    int start;
    int end;
    taskDataTag* taskData;
};

struct shellThreadDataTag {
    int start;
    int end;
    std::vector<int> result;
    shellTaskDataTag* shellTaskData;
};

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
    syntax.addFlag("-mt", "-multiThread", MSyntax::kBoolean);
    return syntax;
}

void combination(int N, std::vector<std::vector<int>>& vec)
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

float FindUvOverlaps::getTriangleArea(float& Ax, float& Ay, float& Bx, float& By, float& Cx, float& Cy)
{
    float area = ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) / (float)2.0;
    return area;
}

bool FindUvOverlaps::checkShellIntersection(UvShell& s1, UvShell& s2)
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
    float u_current, v_current;
    float u_next, v_next;
    float area1, area2;
    float u2 = u + (float)10.0;
    bool isCrossingA;
    bool isCrossingB;
    int polygonVertexCount = (int)uvIds.size();
    int numIntersections = 0;

    for (int currentIndex = 0; currentIndex < polygonVertexCount; currentIndex++) {
        isCrossingA = true;
        isCrossingB = true;
        u_current = uArray[uvIds[currentIndex]];
        v_current = vArray[uvIds[currentIndex]];
        if (currentIndex == polygonVertexCount - 1) {
            u_next = uArray[uvIds[0]];
            v_next = vArray[uvIds[0]];
        } else {
            u_next = uArray[uvIds[currentIndex + 1]];
            v_next = vArray[uvIds[currentIndex + 1]];
        }

        // If parallel edges
        if (v == v_current && v == v_next)
            continue;

        // If upward edges
        if (v_current < v_next)
            if (v == v_next)
                continue;

        // If downward edges
        if (v_current > v_next)
            if (v == v_current)
                continue;

        area1 = getTriangleArea(u, v, u_current, v_current, u2, v);
        area2 = getTriangleArea(u, v, u_next, v_next, u2, v);
        if ((area1 > 0.0 && area2 > 0.0) || (area1 < 0.0 && area2 < 0.0)) {
            isCrossingA = false;
        }
        area1 = getTriangleArea(u_current, v_current, u, v, u_next, v_next);
        area2 = getTriangleArea(u_current, v_current, u2, v, u_next, v_next);
        if ((area1 > 0.0 && area2 > 0.0) || (area1 < 0.0 && area2 < 0.0)) {
            isCrossingB = false;
        }
        if (isCrossingA == true && isCrossingB == true) {
            numIntersections++;
        }
    }

    if ((numIntersections % 2) != 0) {
        return true;
    } else {
        return false;
    }
}

MStatus FindUvOverlaps::createShellTaskData(UvShell& shellA,
    UvShell& shellB,
    std::unordered_map<int, std::vector<int>>& uvMap)
{
    MStatus stat = MThreadPool::init();
    if (MStatus::kSuccess != stat) {
        MString str = MString("Error creating threadpool");
        MGlobal::displayError(str);
        return MS::kFailure;
    }

    shellTaskDataTag shellTaskData;
    shellTaskData.shellA = &shellA;
    shellTaskData.shellB = &shellB;
    shellTaskData.uvMap = &uvMap;

    MThreadPool::newParallelRegion(createShellThreadData, (void*)&shellTaskData);
    MThreadPool::release();

    // Append bad polygons found in each thread to the final result array
    std::unordered_set<int>::iterator resultSetIter;
    for (resultSetIter = shellTaskData.resultIndexSet.begin(); resultSetIter != shellTaskData.resultIndexSet.end(); ++resultSetIter) {
        shellIntersectionsResult.append(*resultSetIter);
    }

    return MS::kSuccess;
}

MStatus FindUvOverlaps::createTaskData(int numPolygons)
{
    MStatus stat = MThreadPool::init();
    if (MStatus::kSuccess != stat) {
        MString str = MString("Error creating threadpool");
        MGlobal::displayError(str);
        return MS::kFailure;
    }

    taskDataTag taskData;
    taskData.start = 0;
    taskData.end = numPolygons - 1;
    taskData.boolArray = new int[numPolygons]();
    MThreadPool::newParallelRegion(createThreadData, (void*)&taskData);

    // pool is reference counted. Release reference to current thread instance
    MThreadPool::release();

    // release reference to whole pool which deletes all threads
    delete[] taskData.boolArray;

    // std::cout << "size of array" << taskData.resultIndexArray.length() << std::endl;
    if (taskData.innerIntersections.length() != 0) {
        for (unsigned int i = 0; i < taskData.innerIntersections.length(); i++) {
            innerIntersectionsResult.copy(taskData.innerIntersections);
        }
    }

    return MS::kSuccess;
}

void FindUvOverlaps::createShellThreadData(void* data, MThreadRootTask* root)
{
    shellTaskDataTag* shellTaskData = (shellTaskDataTag*)data;
    shellThreadDataTag shellThreadData[NUM_TASKS];

    int numBorderPoints = (int)shellTaskData->shellA->borderUvPoints.size();
    int taskLength = (numBorderPoints + NUM_TASKS - 1) / NUM_TASKS;
    int start = 0;
    int end = taskLength;
    int lastTask = NUM_TASKS - 1;

    for (int i = 0; i < NUM_TASKS; ++i) {
        if (i == lastTask) {
            end = numBorderPoints;
        }
        shellThreadData[i].start = start;
        shellThreadData[i].end = end;
        shellThreadData[i].shellTaskData = shellTaskData;

        start += taskLength;
        end += taskLength;

        MThreadPool::createTask(findShellIntersectionsMT, (void*)&shellThreadData[i], root);
    }
    MThreadPool::executeAndJoin(root);

    std::unordered_set<int> resultSet;
    for (int i = 0; i < NUM_TASKS; i++) {
        std::vector<int>& result = shellThreadData[i].result;
        if (result.size() == 0) {
            continue;
        } else {
            std::vector<int>::iterator itVec;
            for (itVec = result.begin(); itVec != result.end(); ++itVec) {
                shellTaskData->resultIndexSet.insert(*itVec);
            }
        }
    }
}

void FindUvOverlaps::createThreadData(void* data, MThreadRootTask* root)
{
    taskDataTag* taskD = (taskDataTag*)data;
    threadDataTag threadData[NUM_TASKS];

    int numFaces = taskD->end + 1;
    int taskLength = (numFaces + NUM_TASKS - 1) / NUM_TASKS;
    int start = 0;
    int end = taskLength;
    int lastTask = NUM_TASKS - 1;

    for (int i = 0; i < NUM_TASKS; ++i) {
        if (i == lastTask) {
            end = numFaces;
        }
        threadData[i].start = start;
        threadData[i].end = end;
        threadData[i].taskData = taskD;

        start += taskLength;
        end += taskLength;

        MThreadPool::createTask(findInnerIntersectionsMT, (void*)&threadData[i], root);
    }

    MThreadPool::executeAndJoin(root);

    for (int i = 0; i < numFaces; i++) {
        if (taskD->boolArray[i] == 1) {
            taskD->innerIntersections.append(i);
        }
    }
}

MThreadRetVal FindUvOverlaps::findShellIntersectionsMT(void* data)
{
    shellThreadDataTag* threadData = (shellThreadDataTag*)data;
    std::vector<int>& borderUVs = threadData->shellTaskData->shellA->borderUvPoints;
    float u, v;
    MFnMesh fnMesh(mDagPath);

    for (int i = threadData->start; i < threadData->end; i++) {
        fnMesh.getUV(borderUVs[i], u, v);

        float& uMin = threadData->shellTaskData->shellB->uMin;
        float& uMax = threadData->shellTaskData->shellB->uMax;
        float& vMin = threadData->shellTaskData->shellB->vMin;
        float& vMax = threadData->shellTaskData->shellB->vMax;

        if (u < uMin || u > uMax) {
            continue;
        }
        if (v < vMin || v > vMax) {
            continue;
        }

        std::unordered_set<int>::iterator polygonIter;
        for (polygonIter = threadData->shellTaskData->shellB->polygonIDs.begin(); polygonIter != threadData->shellTaskData->shellB->polygonIDs.end(); ++polygonIter) {
            bool isInPolygon = checkCrossingNumber(
                u,
                v,
                threadData->shellTaskData->uvMap->operator[](*polygonIter));
            if (isInPolygon == true) {
                threadData->result.push_back(*polygonIter);
                break;
            }
        }
    }

    return (MThreadRetVal)0;
}

MStatus FindUvOverlaps::findShellIntersectionsST(UvShell& shellA,
    UvShell& shellB,
    std::unordered_map<int, std::vector<int>>& uvMap,
    std::vector<bool>& resultBoolVector)
{
    std::vector<int>& borderUVs = shellA.borderUvPoints;
    for (int i = 0; i < borderUVs.size(); i++) {
        float u, v;
        fnMesh.getUV(borderUVs[i], u, v);

        if (u < shellB.uMin || u > shellB.uMax) {
            continue;
        }
        if (v < shellB.vMin || v > shellB.vMax) {
            continue;
        }
        std::unordered_set<int>::iterator polygonIter;
        for (polygonIter = shellB.polygonIDs.begin(); polygonIter != shellB.polygonIDs.end(); ++polygonIter) {
            bool isInPolygon = checkCrossingNumber(u, v, uvMap.operator[](*polygonIter));
            if (isInPolygon == true) {
                resultBoolVector[*polygonIter] = true;
                break;
            }
        }
    }

    return MS::kSuccess;
}

MThreadRetVal FindUvOverlaps::findInnerIntersectionsMT(void* data)
{
    threadDataTag* threadData = (threadDataTag*)data;

    MFnMesh fnMesh(mDagPath);

    int vertexList[3];
    MIntArray vertexIdArray;
    std::unordered_map<int, int> localVtxIdMap;

    for (int faceId = threadData->start; faceId < threadData->end; faceId++) {
        fnMesh.getPolygonVertices(faceId, vertexIdArray);
        int numTriangles = vertexIdArray.length() - 2;

        for (unsigned int localId = 0; localId < vertexIdArray.length(); localId++) {
            localVtxIdMap[vertexIdArray[localId]] = localId;
        }

        for (int triId = 0; triId < numTriangles; triId++) {
            UvPoint uvPointArray[3];
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
                threadData->taskData->boolArray[faceId] = 1;
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

    if (argData.isFlagSet("-multiThread"))
        argData.getFlagArgument("-multiThread", 0, isMultiThreaded);
    else
        isMultiThreaded = false;

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
    double timerResult1;
    double timerResult2;

    // Get basic mesh information
    fnMesh.setObject(mDagPath);
    int numFaces = fnMesh.numPolygons();
    int numUVs = fnMesh.numUVs();
    int numVerts = fnMesh.numVertices();
    MString fullPath = mDagPath.fullPathName();

    // run multithread for self intersection check
    if (verbose) {
        MGlobal::displayInfo("Checking internal self intersections.");
    }

    timer.beginTimer();
    status = createTaskData(numFaces);
    timer.endTimer();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Setup result for self intersections
    for (unsigned int i = 0; i < innerIntersectionsResult.length(); i++) {
        MString index;
        index.set(innerIntersectionsResult[i]);
        MString n = fullPath + ".f[" + index + "]";
        resultStrArray.append(n);
    }

    if (verbose) {
        timerResult1 = timer.elapsedTime();
        MString timeStr;
        timeStr.set(timerResult1);
        MGlobal::displayInfo("Finished self intersection check.");
        MString r = "Result : " + timeStr + " seconds.";
        MGlobal::displayInfo(r);
    }

    MIntArray uvShellIds;
    unsigned int numUVshells;
    fnMesh.getUvShellsIds(uvShellIds, numUVshells);
    if (numUVshells == 1) {
        if (verbose) {
            MGlobal::displayInfo("No multiple shells are found.");
        }
    } else {
        if (verbose) {
            MGlobal::displayInfo("Multiple UV shells are found. Running Shell intersection checks.");
        }
        // Timer for shell intersection check
        timer.beginTimer();

        // Setup uv shell objects
        std::vector<UvShell> uvShellArray;
        uvShellArray.resize(numUVshells);
        for (unsigned int i = 0; i < numUVshells; i++) {
            UvShell shell;
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
                // Get and insert polygon IDs to the shell
                int thisIndex = uvIndexArray[0];
                int shellNumber = uvShellIds[thisIndex];
                itVerts.getConnectedFaces(connectedFacesArray);
                for (unsigned int f = 0; f < connectedFacesArray.length(); f++) {
                    int connectedFaceID = connectedFacesArray[f];
                    uvShellArray[shellNumber].polygonIDs.insert(connectedFaceID);
                }
            } else {
                // If current vertex has multiple UV points, its UVs are on a shell border
                for (unsigned int uvi = 0; uvi < uvIndexArray.length(); uvi++) {
                    int uvIndex = uvIndexArray[uvi];
                    int shellNumber = uvShellIds[uvIndex];
                    uvShellArray[shellNumber].borderUvPoints.push_back(uvIndex);
                }
            }
        }

        // Get UV values
        fnMesh.getUVs(uArray, vArray);

        MIntArray uvCounts;
        MIntArray uvIds;
        fnMesh.getAssignedUVs(uvCounts, uvIds);
        std::unordered_map<int, std::vector<int>> uvMap;
        int counter = 0;
        for (unsigned int i = 0; i < uvCounts.length(); i++) {
            int count = uvCounts[i];
            std::vector<int> uvs(count);
            for (int c = 0; c < count; c++) {
                uvs[c] = uvIds[counter];
                counter++;
            }
            uvMap[i] = uvs;
        }

        for (int i = 0; i < numUVs; i++) {
            UvPoint p;
            p.u = uArray[i];
            p.v = vArray[i];
            p.index = i;
            p.shellIndex = uvShellIds[i];
            uvShellArray[uvShellIds[i]].uvPoints.push_back(p);
            uvShellArray[uvShellIds[i]].uVector.push_back(uArray[i]);
            uvShellArray[uvShellIds[i]].vVector.push_back(vArray[i]);
        }

        // Get min and max for each bounding box
        for (unsigned int i = 0; i < numUVshells; i++) {
            UvShell& shell = uvShellArray[i];
            shell.uMax = *std::max_element(shell.uVector.begin(), shell.uVector.end());
            shell.uMin = *std::min_element(shell.uVector.begin(), shell.uVector.end());
            shell.vMax = *std::max_element(shell.vVector.begin(), shell.vVector.end());
            shell.vMin = *std::min_element(shell.vVector.begin(), shell.vVector.end());
        }

        // comments here
        std::vector<std::vector<int>> shellCombVec;
        combination(numUVshells, shellCombVec);

        // Initialize bool vector for shell intersection result
        resultBoolVector.resize(numFaces);
        std::fill(resultBoolVector.begin(), resultBoolVector.end(), false);

        for (int i = 0; i < shellCombVec.size(); i++) {
            int& shellIndexA = shellCombVec[i][0];
            int& shellIndexB = shellCombVec[i][1];
            UvShell& shellA = uvShellArray[shellIndexA];
            UvShell& shellB = uvShellArray[shellIndexB];

            bool isIntersected = checkShellIntersection(shellA, shellB);

            if (isIntersected == true) {
                if (isMultiThreaded) {
                    status = createShellTaskData(shellA, shellB, uvMap);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                    status = createShellTaskData(shellB, shellA, uvMap);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                } else {
                    status = findShellIntersectionsST(shellA, shellB, uvMap, resultBoolVector);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                    status = findShellIntersectionsST(shellB, shellA, uvMap, resultBoolVector);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                }
            } else {
            }
        }

        timer.endTimer();
        if (verbose) {
            timerResult2 = timer.elapsedTime();
            MString timeStr;
            timeStr.set(timerResult2);
            MGlobal::displayInfo("Finished shell intersection check.");
            MString r = "Result : " + timeStr + " seconds.";
            MGlobal::displayInfo(r);
        }
        if (isMultiThreaded) {
            for (unsigned int i = 0; i < shellIntersectionsResult.length(); i++) {
                MString index;
                index.set(shellIntersectionsResult[i]);
                MString n = fullPath + ".f[" + index + "]";
                resultStrArray.append(n);
            }
        } else {
            MString index;
            for (int x = 0; x < numFaces; x++) {
                if (resultBoolVector[x] == true) {
                    index.set(x);
                    MString n = fullPath + ".f[" + index + "]";
                    resultStrArray.append(n);
                }
            }
        }
    }

    MPxCommand::setResult(resultStrArray);

    if (verbose) {
        double resultTotal = timerResult1 + timerResult2;
        MString resultTotalStr;
        resultTotalStr.set(resultTotal);
        MString r = "Result Total: " + resultTotalStr + " seconds.";
        MGlobal::displayInfo(r);
    }

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
