from maya import OpenMaya
from maya import OpenMayaMPx
import sys


kPluginCmdName = "samplePyCmd"
kVerboseFlag = "-v"
kVerboseLongFlag = "-verbose"


# command
class SamplePyCmd(OpenMayaMPx.MPxCommand):

    def __init__(self):
        super(SamplePyCmd, self).__init__()

        self.verbose = False
        self.cmdArg = "Initial arg"

    def doIt(self, args):

        # Parse the arguments.
        argData = OpenMaya.MArgDatabase(self.syntax(), args)
        try:
            self.cmdArg = argData.commandArgumentString(0)
        except RuntimeError:
            pass
        if argData.isFlagSet(kVerboseFlag):
            self.verbose = argData.flagArgumentBool(kVerboseFlag, 0)

        # Do something
        print "Hello world"

        self.undoIt

    def undoIt(self):

        if self.verbose:
            print 'verbose mode'
            print self.cmdArg


# Creator
def cmdCreator():
    # Create the command
    return OpenMayaMPx.asMPxPtr(SamplePyCmd())


# Syntax creator
def syntaxCreator():
    syntax = OpenMaya.MSyntax()
    syntax.addArg(OpenMaya.MSyntax.kString)
    syntax.addFlag(kVerboseFlag, kVerboseLongFlag, OpenMaya.MSyntax.kBoolean)
    return syntax


# Initialize the script plug-in
def initializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject, "Name", "1.0", "Any")
    try:
        mplugin.registerCommand(kPluginCmdName, cmdCreator, syntaxCreator)
    except:
        sys.stderr.write("Failed to register command: %s\n" % kPluginCmdName)
        raise


# Uninitialize the script plug-in
def uninitializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.deregisterCommand(kPluginCmdName)
    except:
        sys.stderr.write("Failed to unregister command: %s\n" % kPluginCmdName)
        raise
