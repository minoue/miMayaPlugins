#ifndef __MESHCHECKER_H__
#define __MESHCHECKER_H__

#include <maya/MDagPath.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPxCommand.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>

#include <string>

class MeshChecker : public MPxCommand {
public:
    MeshChecker();
    virtual ~MeshChecker();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creator();
    static MSyntax newSyntax();

    // check functions
    MStatus findTriangles();
    MStatus findNgons();
    MStatus findNonManifoldEdges();
    MStatus findLaminaFaces();
    MStatus findBiValentFaces();
    MStatus findZeroAreaFaces(double& faceAreaMax);
    MStatus findMeshBorderEdges();
    MStatus findCreaseEDges();

    MStringArray setResultString(std::string componentType);

    enum Check {
        TRIANGLES,
        NGONS,
        NON_MANIFOLD_EDGES,
        LAMINA_FACES,
        BI_VALENT_FACES,
        ZERO_AREA_FACES,
        MESH_BORDER,
        CREASE_EDGE,
        TEST
    };

private:
    MDagPath mDagPath;
    double faceAreaMax;
    MIntArray indexArray;
    unsigned int checkNumber;
    MStringArray resultArray;
};

#endif /* defined(__MESHCHECKER_H__) */
