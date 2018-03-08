#include "meshChecker.h"
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>
#include <maya/MUintArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MPlug.h>


MeshChecker::MeshChecker()
{
}

MeshChecker::~MeshChecker()
{
}

MStatus MeshChecker::findTriangles()
{
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.polygonVertexCount() == 3) {
            indexArray.append(mItPoly.index());
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findNgons()
{
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.polygonVertexCount() >= 5) {
            indexArray.append(mItPoly.index());
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findNonManifoldEdges()
{
    MStatus status;
    int faceCount;
    for (MItMeshEdge mItEdge(mDagPath); !mItEdge.isDone(); mItEdge.next()) {
        status = mItEdge.numConnectedFaces(faceCount);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        std::cout << faceCount << std::endl;
        if (faceCount >= 3) {
            indexArray.append(mItEdge.index());
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findLaminaFaces()
{
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.isLamina() == true) {
            indexArray.append(mItPoly.index());
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findBiValentFaces()
{
    MIntArray connectedFaces;
    MIntArray connectedEdges;
    for (MItMeshVertex mItVert(mDagPath); !mItVert.isDone(); mItVert.next()) {
        mItVert.getConnectedFaces(connectedFaces);
        mItVert.getConnectedEdges(connectedEdges);
        int numFaces = connectedFaces.length();
        int numEdges = connectedEdges.length();
        if (numFaces == 2 && numEdges == 2) {
            indexArray.append(mItVert.index());
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findZeroAreaFaces(double& faceAreaMax)
{
    double area;
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        mItPoly.getArea(area);
        if (area < faceAreaMax) {
            indexArray.append(mItPoly.index());
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findMeshBorderEdges()
{
    for (MItMeshEdge mItEdge(mDagPath); !mItEdge.isDone(); mItEdge.next()) {
        bool isBorder = mItEdge.onBoundary();
        if (isBorder) {
            indexArray.append(mItEdge.index());
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findCreaseEDges()
{
    MFnMesh fnMesh(mDagPath);
    MUintArray edgeIds;
    MDoubleArray creaseData;
    fnMesh.getCreaseEdges(edgeIds, creaseData);

    if (edgeIds.length() != 0) {
        for (unsigned int i = 0; i < edgeIds.length(); i++) {
            if (creaseData[i] == 0)
                continue;
            int edgeId = (int)edgeIds[i];
            indexArray.append(edgeId);
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findZeroLengthEdges() {
    double length;
    for (MItMeshEdge mItEdge(mDagPath); !mItEdge.isDone(); mItEdge.next()) {
        mItEdge.getLength(length);
        if (length < minEdgeLength) {
            indexArray.append(mItEdge.index());
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findUnfrozenVertices() {
    // reference
    // https://nccastaff.bournemouth.ac.uk/jmacey/RobTheBloke/www/research/maya/mfn_attributes.htm
    mDagPath.extendToShape();
    MFnDagNode mFnDag(mDagPath);
    MPlug pntsArray = mFnDag.findPlug("pnts");
    
    MFnMesh fnMesh(mDagPath);
    unsigned int numVerts = fnMesh.numVertices();

    for (unsigned int i = 0; i < numVerts; i++) {
        // MPlug compound = pntsArray.elementByPhysicalIndex(i);
        MPlug compound = pntsArray.elementByLogicalIndex(i);
        float x, y, z;
        if (compound.isCompound()) {
            MPlug plug_x = compound.child(0);
            MPlug plug_y = compound.child(1);
            MPlug plug_z = compound.child(2);
        
            plug_x.getValue(x);
            plug_y.getValue(y);
            plug_z.getValue(z);
            
            float temp = x + y + z;
            if (temp != 0)
                indexArray.append(i);
        }
    }
    return MS::kSuccess;
}

MStringArray MeshChecker::setResultString(std::string componentType)
{
    MString fullpath = mDagPath.fullPathName();
    MStringArray resultStringArray;
    int index;
    for (unsigned int i = 0; i < indexArray.length(); i++) {
        index = indexArray[i];
        if (componentType == "face") {
            MString name = fullpath + ".f[" + index + "]";
            resultStringArray.append(name);
        } else if (componentType == "vertex") {
            MString name = fullpath + ".vtx[" + index + "]";
            resultStringArray.append(name);
        } else if (componentType == "edge") {
            MString name = fullpath + ".e[" + index + "]";
            resultStringArray.append(name);
        } else if (componentType == "uv") {
            MString name = fullpath + ".map[" + index + "]";
            resultStringArray.append(name);
        }
    }
    return resultStringArray;
}

MSyntax MeshChecker::newSyntax()
{
    MSyntax syntax;
    syntax.addFlag("-c", "-check", MSyntax::kUnsigned);
    syntax.addFlag("-fa", "-faceAreaMax", MSyntax::kDouble);
    syntax.addFlag("-mel", "-minEdgeLength", MSyntax::kDouble);
    return syntax;
}

MStatus MeshChecker::doIt(const MArgList& args)
{
    MStatus status;

    if (args.length() != 1) {
        MGlobal::displayError("Need 1 arg!");
        return MStatus::kFailure;
    }

    MArgDatabase argData(syntax(), args);

    // arg
    MString argument = args.asString(0, &status);
    if (status != MS::kSuccess) {
        return MStatus::kFailure;
    }
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (argData.isFlagSet("-check")) {
        argData.getFlagArgument("-check", 0, checkNumber);
    } else {
        MGlobal::displayError("Check type required.");
        return MS::kFailure;
    }

    if (argData.isFlagSet("-faceAreaMax"))
        argData.getFlagArgument("-faceAreaMax", 0, faceAreaMax);
    else
        faceAreaMax = 0.00001;

    if (argData.isFlagSet("-minEdgeLength"))
        argData.getFlagArgument("-minEdgeLength", 0, minEdgeLength);
    else
        minEdgeLength = 0.000001;

    MSelectionList mList;
    mList.add(argument);
    mList.getDagPath(0, mDagPath);

    switch (checkNumber) {
    case MeshChecker::TRIANGLES:
        status = findTriangles();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString("face");
        break;
    case MeshChecker::NGONS:
        status = findNgons();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString("face");
        break;
    case MeshChecker::NON_MANIFOLD_EDGES:
        status = findNonManifoldEdges();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString("edge");
        break;
    case MeshChecker::LAMINA_FACES:
        status = findLaminaFaces();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString("face");
        break;
    case MeshChecker::BI_VALENT_FACES:
        status = findBiValentFaces();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString("vertex");
        break;
    case MeshChecker::ZERO_AREA_FACES:
        status = findZeroAreaFaces(faceAreaMax);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString("face");
        break;
    case MeshChecker::MESH_BORDER:
        status = findMeshBorderEdges();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString("edge");
        break;
    case MeshChecker::CREASE_EDGE:
        status = findCreaseEDges();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString("edge");
        break;
    case MeshChecker::ZERO_LENGTH_EDGES:
        status = findZeroLengthEdges();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        resultArray = setResultString("edge");
        break;
    case MeshChecker::UNFROZEN_VERTICES:
        status = findUnfrozenVertices();
        CHECK_MSTATUS_AND_RETURN_IT(status);
        // if (unfrozen) {
        //     resultArray.append(mDagPath.fullPathName());
        // }
        resultArray = setResultString("vertex");
        break;
    case MeshChecker::TEST:
        break;
    default:
        MGlobal::displayError("Invalid check number");
        return MS::kFailure;
        break;
    }

    MPxCommand::setResult(resultArray);

    return redoIt();
}

MStatus MeshChecker::redoIt()
{
    return MS::kSuccess;
}

MStatus MeshChecker::undoIt()
{
    return MS::kSuccess;
}

bool MeshChecker::isUndoable() const
{
    return false;
}

void* MeshChecker::creator()
{
    return new MeshChecker;
}
