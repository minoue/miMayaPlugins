""" docstring """

import pickle
from maya import OpenMaya
from maya import OpenMayaUI
from maya import OpenMayaAnim
from maya import cmds
from PySide2 import QtWidgets, QtCore
import shiboken2


def getClusterFromMesh():
    """ docstring """

    sel = OpenMaya.MSelectionList()
    OpenMaya.MGlobal.getActiveSelectionList(sel)
    dagPath = OpenMaya.MDagPath()
    sel.getDagPath(0, dagPath)
    dagPath.extendToShape()

    conns = cmds.listConnections(dagPath.fullPathName())
    clusters = [i for i in conns if cmds.objectType(i) == "cluster"]

    if len(clusters) == 0:
        cmds.warning("No clusters found")
        return

    firstCluster = clusters[0]

    return firstCluster


def getClusterWeights(meshName, clusterName=None):
    """ asdf """

    sel = OpenMaya.MSelectionList()
    sel.add(meshName)

    if clusterName is None:
        sel.add(getClusterFromMesh())
    else:
        sel.add(clusterName)

    dagPath = OpenMaya.MDagPath()
    mObj = OpenMaya.MObject()
    sel.getDagPath(0, dagPath)
    sel.getDependNode(1, mObj)

    fn_c = OpenMaya.MFnSingleIndexedComponent()
    c_obj = fn_c.create(OpenMaya.MFn.kMeshVertComponent)

    fnMesh = OpenMaya.MFnMesh(dagPath)
    numVerts = fnMesh.numVertices()

    indexArray = OpenMaya.MIntArray()

    for i in range(numVerts):
        indexArray.append(i)
    fn_c.addElements(indexArray)

    cluster = OpenMayaAnim.MFnWeightGeometryFilter()
    cluster.setObject(mObj)
    floatArray = OpenMaya.MFloatArray()
    cluster.getWeights(dagPath, c_obj, floatArray)

    return floatArray

def setClusterWeights(meshName, weights, clusterName=None):
    """ asdf """

    sel = OpenMaya.MSelectionList()
    sel.add(meshName)

    if clusterName is None:
        sel.add(getClusterFromMesh())
    else:
        sel.add(clusterName)

    dagPath = OpenMaya.MDagPath()
    mObj = OpenMaya.MObject()
    sel.getDagPath(0, dagPath)
    sel.getDependNode(1, mObj)

    fn_c = OpenMaya.MFnSingleIndexedComponent()
    c_obj = fn_c.create(OpenMaya.MFn.kMeshVertComponent)

    fnMesh = OpenMaya.MFnMesh(dagPath)
    numVerts = fnMesh.numVertices()

    indexArray = OpenMaya.MIntArray()

    for i in range(numVerts):
        indexArray.append(i)
    fn_c.addElements(indexArray)

    cluster = OpenMayaAnim.MFnWeightGeometryFilter()
    cluster.setObject(mObj)
    cluster.setWeight(dagPath, c_obj, weights)


def readSymData(path):
    """ Read sym data file """

    try:
        with open(path, 'rb') as handle:
            data = pickle.load(handle)
        return data
    except IOError:
        print "No such file"


class Window(QtWidgets.QWidget):
    """ class document """

    def __init__(self, parent=None):
        super(Window, self).__init__(parent)

        self.setWindowTitle("Mirror Cluster Weights")
        self.setWindowFlags(QtCore.Qt.Window)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)

        self.lineEdit = QtWidgets.QLineEdit()
        self.setButton = QtWidgets.QPushButton("Set")
        self.setButton.clicked.connect(self.setData)
        self.mirrorButton = QtWidgets.QPushButton("Mirror")
        self.mirrorButton.clicked.connect(self.mirror)

        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(self.lineEdit)
        layout.addWidget(self.setButton)
        layout.addWidget(self.mirrorButton)

        self.setLayout(layout)


    def setData(self):
        """ Read sym data file """

        options = QtWidgets.QFileDialog.DontUseNativeDialog
        fileName, _ = QtWidgets.QFileDialog.getOpenFileName(
            self,
            "QFileDialog.getOpenFileName()",
            "",
            "All Files (*);;JSON Files (*.json)",
            options=options)

        if fileName:
            try:
                # with open(fileName, 'rb') as handle:
                #     data = pickle.load(handle)
                self.lineEdit.setText(fileName)
            except IOError:
                print "No such file"

    def mirror(self):
        """ mirror """

        sel = cmds.ls(sl=True, fl=True, long=True)[0]
        data = readSymData(self.lineEdit.text())

        weights = getClusterWeights(sel)

        for leftIndex in data:
            rightIndex = data[leftIndex]
            leftValue = weights[leftIndex]
            weights.set(leftValue, rightIndex)

        setClusterWeights(sel, weights)


def getMayaWindow():
    """ Get maya main window """

    ptr = OpenMayaUI.MQtUtil.mainWindow()
    return shiboken2.wrapInstance(long(ptr), QtWidgets.QMainWindow)


def main():
    """ main """

    w = Window(getMayaWindow())
    w.show()


if __name__ == "__main__":
    main()
