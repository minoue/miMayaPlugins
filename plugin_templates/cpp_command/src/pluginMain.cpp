#include <maya/MFnPlugin.h>
#include "sampleCommand.h"


MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "0.0.1", "Any");
    fnPlugin.registerCommand("sampleCommand", SampleCommand::creator);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("sampleCommand");
    return MS::kSuccess;
}
