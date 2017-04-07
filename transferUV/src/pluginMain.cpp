#include <maya/MFnPlugin.h>
#include "transferUV.h"


MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "0.0.1", "Any");
    fnPlugin.registerCommand("transferUV", TransferUV::creator, TransferUV::newSyntax);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("transferUV");
    return MS::kSuccess;
}                               
