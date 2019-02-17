// ----------------------------------------------------------------------------
// "THE BEER-WARE LICENSE" (Revision 42):
// <michitaka_inoue@icloud.com> wrote this file.  As long as you retain this notice you
// can do whatever you want with this stuff. If we meet some day, and you think
// this stuff is worth it, you can buy me a beer in return.   Michitaka Inoue
// ----------------------------------------------------------------------------
//

#include "snapToClosest.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject mObj)
{
    MStatus status;

    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "1.1.0", "Any");

    status = fnPlugin.registerCommand(
        "snapToClosest",
        SnapToClosest::creator,
        SnapToClosest::newSyntax);

    CHECK_MSTATUS_AND_RETURN_IT(status);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("snapToClosest");
    return MS::kSuccess;
}
