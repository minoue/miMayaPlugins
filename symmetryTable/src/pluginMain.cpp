#include <maya/MFnPlugin.h>
#include "symmetryTable.h"
#include <string>


MStatus initializePlugin(MObject mObj)
{
    std::string version_str("0.0.2");
    std::string version_date(__DATE__);
    std::string version_time(__TIME__);
    std::string ver = version_str + " / " + version_date + " / " + version_time;

    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", ver.c_str(), "Any");
    fnPlugin.registerCommand("buildSymmetryTable", SymmetryTable::creater);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("buildSymmetryTable");
    return MS::kSuccess;
}
