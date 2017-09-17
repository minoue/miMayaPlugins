#ifndef __TOPOLOGYCHECKER_H__
#define __TOPOLOGYCHECKER_H__

#include <maya/MDagPath.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPxCommand.h>
#include <maya/MStringArray.h>
#include <string>

class TopologyChecker : public MPxCommand {
public:
    TopologyChecker();
    virtual ~TopologyChecker();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creator();
    static MSyntax newSyntax();

    // check functions
    MStatus findTriangles(MIntArray& indexArray);
    MStatus findNgons(MIntArray& indexArray);
    MStatus findNonManifoldEdges(MIntArray& indexArray);
    MStatus findLaminaFaces(MIntArray& indexArray);
    MStatus findBiValentFaces(MIntArray& indexArray);
    MStatus findZeroAreaFaces(MIntArray& indexArray,
        double& faceAreaMax);
    MStringArray setResultString(MIntArray& indexArray,
        std::string componentType);

    enum Check {
        TRIANGLES,
        NGONS,
        NON_MANIFOLD_EDGES,
        LAMINA_FACES,
        BI_VALENT_FACES,
        ZERO_AREA_FACES,
        TEST
    };

private:
    MDagPath mDagPath;
    double faceAreaMax;
};

#endif /* defined(__TOPOLOGYCHECKER_H__) */
