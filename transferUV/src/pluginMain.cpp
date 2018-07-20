// pluginMain.cpp
//
// Copyright (c) 2018 Michitaka Inoue
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include "transferUV.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", "1.0.3", "Any");
    fnPlugin.registerCommand("transferUV", TransferUV::creator, TransferUV::newSyntax);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("transferUV");
    return MS::kSuccess;
}
