""" Yet another UV transfer command """

import sys
from maya.api import OpenMaya


kPluginCmdName = "transferUV"
kPluginVersion = "0.0.1"
kPluginAuthor = "Michi Inoue"
kSourceUvSetFlag = "-suv"
kSourceUvSetFlagLong = "-sourceUvSet"
kTargetUvSetFlag = "-tuv"
kTargetUvSetFlagLong = "-targetUvSet"
kSourceMeshFlag = "-sm"
kSourceMeshFlagLong = "-sourceMesh"
kTargetMeshFlag = "-tm"
kTargetMeshFlagLong = "-targetMesh"


class TransferUV(OpenMaya.MPxCommand):

    def __init__(self):
        super(TransferUV, self).__init__()

        self.sourceMesh = None
        self.targetMesh = None
        self.sourceUvSet = None
        self.targetUvSet = None

    def doIt(self, args):

        # Parse the arguments.
        argData = OpenMaya.MArgDatabase(self.syntax(), args)
        try:
            self.cmdArg = argData.commandArgumentString(0)
        except RuntimeError:
            pass

        if argData.isFlagSet(kSourceMeshFlag):
            argList = argData.getFlagArgumentList(kSourceMeshFlag, 0)
            self.sourceMesh = argList.asString(0)

        if argData.isFlagSet(kTargetMeshFlag):
            argList = argData.getFlagArgumentList(kTargetMeshFlag, 0)
            self.targetMesh = argList.asString(0)

        if argData.isFlagSet(kSourceUvSetFlag):
            argList = argData.getFlagArgumentList(kSourceUvSetFlag, 0)
            self.sourceUvSet = argList.asString(0)

        if argData.isFlagSet(kTargetUvSetFlag):
            argList = argData.getFlagArgumentList(kTargetUvSetFlag, 0)
            self.targetUvSet = argList.asString(0)

        if self.sourceUvSet is None:
            self.sourceUvSet = "map1"

        if self.targetUvSet is None:
            self.targetUvSet = "map1"

        mSel = OpenMaya.MGlobal.getActiveSelectionList()

        if self.sourceMesh is None:
            try:
                self.sourceDagPath = mSel.getDagPath(0)
            except RuntimeError:
                OpenMaya.MGlobal.displayWarning(
                    "Failed to set source DagPath. Select source mesh first, then target mesh")
                return
        else:
            try:
                mSel.add(self.sourceMesh)
                self.sourceDagPath = mSel.getDagPath(0)
            except RuntimeError:
                OpenMaya.MGlobal.displayWarning("Object doesn't exist")
                return

        if self.targetMesh is None:
            try:
                self.targetDagPath = mSel.getDagPath(1)
            except RuntimeError:
                OpenMaya.MGlobal.displayWarning(
                    "Failed to set target DagPath. Select source mesh first, then target mesh")
                return
        else:
            try:
                mSel.add(self.targetMesh)
                self.targetDagPath = mSel.getDagPath(1)
            except RuntimeError:
                OpenMaya.MGlobal.displayWarning("Object doesn't exist")
                return

        print "Copying uv from {} to {}".format(
            self.sourceDagPath.fullPathName(), self.targetDagPath.fullPathName())

        self.redoIt()

    def redoIt(self):

        sourceFnMesh = OpenMaya.MFnMesh(self.sourceDagPath)
        targetFnMesh = OpenMaya.MFnMesh(self.targetDagPath)

        sourceUvCounts = OpenMaya.MIntArray()
        sourceUvIds = OpenMaya.MIntArray()

        sourceUarray, sourceVarray = sourceFnMesh.getUVs(self.sourceUvSet)
        self.originalUarray, self.originalVarray = targetFnMesh.getUVs(
            self.targetUvSet)

        sourceUvCounts, sourceUvIds = sourceFnMesh.getAssignedUVs()
        self.originalUvCounts, self.originalUvIds = targetFnMesh.getAssignedUVs()

        # Clear target UVs before transfer in order to shrink the uvs array if number of source UVs
        # are smaller than that of target UVs
        targetFnMesh.clearUVs(self.targetUvSet)

        targetFnMesh.setUVs(sourceUarray, sourceVarray, self.targetUvSet)
        targetFnMesh.assignUVs(sourceUvCounts, sourceUvIds, self.targetUvSet)

    def undoIt(self):
        undoMesh = OpenMaya.MFnMesh(self.targetDagPath)

        # Clear target UVs before transfer in order to shrink the uvs array if number of source UVs
        # are smaller than that of target UVs
        undoMesh.clearUVs(self.targetUvSet)
        undoMesh.setUVs(self.originalUarray,
                        self.originalVarray, self.targetUvSet)
        undoMesh.assignUVs(self.originalUvCounts,
                           self.originalUvIds, self.targetUvSet)

    def isUndoable(self):
        return True

    @staticmethod
    def cmdCreator():
        return TransferUV()


def syntaxCreator():
    """ Syntax creator

    Return:
        syntax (OpenMaya.MSyntax): return value

    """
    syntax = OpenMaya.MSyntax()
    syntax.addArg(OpenMaya.MSyntax.kString)
    syntax.addFlag(kSourceUvSetFlag, kSourceUvSetFlagLong,
                   OpenMaya.MSyntax.kString)
    syntax.addFlag(kTargetUvSetFlag, kTargetUvSetFlagLong,
                   OpenMaya.MSyntax.kString)
    syntax.addFlag(kSourceMeshFlag, kSourceMeshFlagLong,
                   OpenMaya.MSyntax.kString)
    syntax.addFlag(kTargetMeshFlag, kTargetMeshFlagLong,
                   OpenMaya.MSyntax.kString)
    return syntax


def maya_useNewAPI():
    """
    The presence of this function tells Maya that the plugin produces, and
    expects to be passed, objects created using the Maya Python API 2.0.
    """
    pass


def initializePlugin(mObject):
    """ Initialize the script plug-in

    Args:
        mObject (OpenMaya.MObject):

    """
    mplugin = OpenMaya.MFnPlugin(mObject, kPluginAuthor, kPluginVersion, "Any")
    try:
        mplugin.registerCommand(
            kPluginCmdName, TransferUV.cmdCreator, syntaxCreator)
    except:
        sys.stderr.write("Failed to register command: %s\n" % kPluginCmdName)
        raise


def uninitializePlugin(mObject):
    """ Uninitialize the script plug-in

    Args:
        mObject (OpenMaya.MObject):

    """
    mplugin = OpenMaya.MFnPlugin(mObject)
    try:
        mplugin.deregisterCommand(kPluginCmdName)
    except:
        sys.stderr.write("Failed to unregister command: %s\n" % kPluginCmdName)
        raise
