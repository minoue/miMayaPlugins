#include "uvChecker.h"
#include "findUvOverlaps.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject mObj)
{
    MFnPlugin ovPlugin(mObj, "Michitaka Inoue", "0.1.0", "Any");
    ovPlugin.registerCommand("findUvOverlaps", FindUvOverlaps::creater, FindUvOverlaps::newSyntax);

    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "0.0.1", "Any");
    fnPlugin.registerCommand("checkUV", UvChecker::creater, UvChecker::newSyntax);

    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin ovPlugin(mObj);
    ovPlugin.deregisterCommand("findUvOverlaps");
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("checkUV");

    return MS::kSuccess;
}
