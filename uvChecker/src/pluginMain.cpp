#include "findUvOverlaps.h"
#include "findUvOverlaps2.h"
#include "uvChecker.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject mObj)
{
    MFnPlugin ovPlugin(mObj, "Michitaka Inoue", "0.1.0", "Any");
    ovPlugin.registerCommand("findUvOverlaps", FindUvOverlaps::creator, FindUvOverlaps::newSyntax);

    MFnPlugin ov2Plugin(mObj, "Michitaka Inoue", "0.1.0", "Any");
    ovPlugin.registerCommand("findUvOverlaps2", FindUvOverlaps2::creator, FindUvOverlaps2::newSyntax);

    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "0.0.1", "Any");
    fnPlugin.registerCommand("checkUV", UvChecker::creator, UvChecker::newSyntax);

    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin ovPlugin(mObj);
    ovPlugin.deregisterCommand("findUvOverlaps");
    MFnPlugin ov2Plugin(mObj);
    ovPlugin.deregisterCommand("findUvOverlaps2");
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("checkUV");

    return MS::kSuccess;
}
