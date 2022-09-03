from maya.api import OpenMaya
import sys


__VERSION__ = "0.1"
__AUTHOR__ = "NAME"


kPluginCmdName = "samplePyCmd"
kVerboseFlag = "-v"
kVerboseLongFlag = "-verbose"


class SamplePyCmd(OpenMaya.MPxCommand):

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

        self.redoIt()

    def redoIt(self):
        # Do something
        print("Hello world")

        if self.verbose:
            print('verbose mode')
            print("Arg : {}".format(self.cmdArg))

    def undoIt(self):
        pass

    def isUndoable(self):
        return True


def cmdCreator():
    return SamplePyCmd()


def syntaxCreator():
    """ Syntax creator

    Return:
        syntax (OpenMaya.MSyntax): return value

    """
    syntax = OpenMaya.MSyntax()
    syntax.addArg(OpenMaya.MSyntax.kString)
    syntax.addFlag(kVerboseFlag, kVerboseLongFlag, OpenMaya.MSyntax.kBoolean)
    return syntax


def initializePlugin(mObj):
    """ Initialize the script plug-in

    Args:
        mobject (OpenMaya.MObject):

    """
    mplugin = OpenMaya.MFnPlugin(mObj, __AUTHOR__, __VERSION__)
    try:
        mplugin.registerCommand(kPluginCmdName, cmdCreator, syntaxCreator)
    except Exception:
        sys.stderr.write("Failed to register command: %s\n" % kPluginCmdName)
        raise


def uninitializePlugin(mObj):
    """ Uninitialize the script plug-in

    Args:
        mobject (OpenMaya.MObject):

    """
    mplugin = OpenMaya.MFnPlugin(mObj)
    try:
        mplugin.deregisterCommand(kPluginCmdName)
    except Exception:
        sys.stderr.write("Failed to unregister command: %s\n" % kPluginCmdName)
        raise


def maya_useNewAPI():
    pass
