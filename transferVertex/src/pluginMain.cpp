#include <maya/MFnPlugin.h>
#include "transferVertex.h"

MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "0.1.0", "Any");
    fnPlugin.registerCommand("transferVertex", TransferVertex::creator, TransferVertex::newSyntax);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("transferVertex");
    return MS::kSuccess;
}
