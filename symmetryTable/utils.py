from __future__ import print_function
import pickle
from maya import cmds


PLUGIN_PATH = "/job/modelling/dev/sandbox/sandbox_minoue/dev/symTable/build/symmetryTable.so"
DATA_PATH = "/net/homes/minoue/symData"


def loadPlugin():
    """ load plugin """

    if not cmds.pluginInfo("symmetryTable", q=True, loaded=True):
        try:
            cmds.loadPlugin(PLUGIN_PATH)
            print("Plugin loaded successfully")
        except RuntimeError:
            cmds.error("Error loading plugin")
    else:
        print("plugin is already loaded")


def write():
    """ write sym table to a file """

    if len(cmds.ls(sl=True)) == 0:
        cmds.warning("Nothing is selected")
        return

    result = cmds.buildSymmetryTable()

    # Convret Long to Int
    pp = [int(i) for i in result]

    # Split two ints to a dict
    # eg. [x, x, x, x, x, x, ...]
    # to..
    # {x: x, x: x, x: x, ...}
    # Key is left verts and values are right verts
    pDict = {}
    pList = [pp[i:i+2] for i in range(0, len(pp), 2)]
    for i in pList:
        pDict[i[0]] = i[1]

    # Store data (serialize)
    with open(DATA_PATH, 'wb') as handle:
        pickle.dump(pDict, handle, protocol=pickle.HIGHEST_PROTOCOL)


def main():
    """ main """

    write()


if __name__ == "__main__":
    main()
