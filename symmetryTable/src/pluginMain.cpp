#include <maya/MFnPlugin.h>
#include "symmetryTable.h"


MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "0.0.1", "Any");
    fnPlugin.registerCommand("buildSymmetryTable", SymmetryTable::creater);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("buildSymmetryTable");
    return MS::kSuccess;
}
