#include "transferUV.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "1.0.0", "Any");
    fnPlugin.registerCommand("transferUV", TransferUV::creator, TransferUV::newSyntax);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("transferUV");
    return MS::kSuccess;
}
