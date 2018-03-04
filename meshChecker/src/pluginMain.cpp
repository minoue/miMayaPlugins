#include "topologyChecker.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "0.0.1", "Any");
    fnPlugin.registerCommand("checkMesh", MeshChecker::creator, MeshChecker::newSyntax);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("checkMesh");
    return MS::kSuccess;
}
