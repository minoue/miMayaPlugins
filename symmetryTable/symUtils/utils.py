import pickle
from maya import cmds
from pymel import core as pm
from functools import partial


PLUGIN_NAME = "symmetryTable.so"


def loadPlugin():
    """ load plugin """

    if not cmds.pluginInfo("symmetryTable", q=True, loaded=True):
        try:
            cmds.loadPlugin(PLUGIN_NAME)
            print("Plugin loaded successfully")
        except RuntimeError:
            cmds.error("Error loading plugin")
    else:
        print("plugin is already loaded")


def createSymData(*args):
    """ write sym table to a file """

    # Get file path
    filepath = pm.fileDialog2(fileMode=0, caption="Save sym data")
    if filepath is None:
        return
    filepath = filepath[0]

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
    with open(filepath, 'wb') as handle:
        pickle.dump(pDict, handle, protocol=pickle.HIGHEST_PROTOCOL)


def getBlendShapeNodes(*args):
    sel = pm.ls(sl=True, fl=True)

    blendShapes = []

    if sel:
        sel = sel[0]
        hist = pm.listHistory(sel)
        for h in hist:
            if isinstance(h, pm.nt.BlendShape):
                blendShapes.append(h)
    
    return blendShapes


def mirrorBlendShapeWeights(*args):
    TARGET = "blendShape1.inputTarget[0].baseWeights"

    textField = args[0]
    symDataPath = textField.getText()

    numVerts = cmds.polyEvaluate(v=True)
    plug = OpenMaya.MPlug()
    sel = OpenMaya.MSelectionList()
    sel.add(TARGET)
    sel.getPlug(0, plug)

    weights = []

    p = OpenMaya.MPlug()
    for i in range(numVerts):
        p = plug.elementByLogicalIndex(i)
        value = p.asFloat()
        weights.append(value)

    try:
        with open(symDataPath, 'rb') as handle:
            unserialized_data = pickle.load(handle)

    except IOError:
        print("No such file")
        return

    for i in unserialized_data:
        leftIndex = i
        leftWeight = weights[leftIndex]

        rightIndex = unserialized_data[leftIndex]
        weights[rightIndex] = leftWeight

    for n, i in enumerate(weights):
        p = OpenMaya.MPlug()
        p = plug.elementByLogicalIndex(n)
        p.setFloat(i)


def gui():
    windowName = "symTools"
    if pm.window(windowName, q=True, exists=True):
        pm.deleteUI(windowName)

    win = pm.window(windowName, title="SymTools")
    pm.columnLayout(adj=True)
    pm.button("Create Sym Data", c=createSymData)
    pm.text("Blend shapes")
    symDataPath = pm.textField()
    pm.button(
        "Mirror blendshape weights",
        command=partial(
            mirrorBlendShapeWeights,
            symDataPath
        ))
    win.show()


def main():
    """ main """

    loadPlugin()
    gui()


if __name__ == "__main__":
    main()
