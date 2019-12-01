// pluginMain.cpp
//
// Copyright (c) 2018 Michitaka Inoue
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include "transferUV.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

MStatus initializePlugin(MObject mObj)
{
    MString date = MGlobal::executeCommandStringResult("date -d;");
    MString version("1.0.5 / ");
    MString ver = version + date;
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", ver.asChar(), "Any");
    fnPlugin.registerCommand("transferUV", TransferUV::creator, TransferUV::newSyntax);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("transferUV");
    return MS::kSuccess;
}
