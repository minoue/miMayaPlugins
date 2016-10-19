#include <maya/MFnPlugin.h>
#include "randomPointOnMesh.h"


MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "0.0.1", "Any");
    fnPlugin.registerCommand("randomPointOnMesh", RandomPointOnMesh::creater);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("randomPointOnMesh");
    return MS::kSuccess;
}
