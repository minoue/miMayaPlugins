// pluginMain.cpp
//
// Copyright (c) 2018 Michitaka Inoue
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include "transferUV.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <string>


MStatus initializePlugin(MObject mObj)
{
    std::string version_str("1.1.0");
    std::string compile_date_str(__DATE__);
    std::string compile_time_str(__TIME__);
    std::string v(version_str + " / " + compile_date_str + " / " + compile_time_str);
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", v.c_str(), "Any");
    fnPlugin.registerCommand("transferUV", TransferUV::creator, TransferUV::newSyntax);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("transferUV");
    return MS::kSuccess;
}
