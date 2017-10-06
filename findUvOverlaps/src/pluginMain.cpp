//
//  pluginMain.cpp
//  threadTestCmd
//
//  Created by Michitaka Inoue on 01/10/2017.
//
//

#include "findUvOverlaps.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "0.0.1", "Any");
    fnPlugin.registerCommand("findUvOverlaps", FindUvOverlaps::creater, FindUvOverlaps::newSyntax);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("findUvOverlaps");
    return MS::kSuccess;
}
