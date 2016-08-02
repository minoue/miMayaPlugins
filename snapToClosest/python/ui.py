from PySide import QtGui, QtCore
from maya import OpenMayaUI
from maya import cmds
import shiboken


def getMayaWindow():
    ptr = OpenMayaUI.MQtUtil.mainWindow()
    return shiboken.wrapInstance(long(ptr), QtGui.QMainWindow)


class SnapWindow(QtGui.QWidget):

    def __init__(self, parent=getMayaWindow()):
        super(SnapWindow, self).__init__(parent)

        self.setWindowTitle("Snap")
        self.setWindowFlags(QtCore.Qt.Tool)

        self.createUI()
        self.layoutUI()

    def createUI(self):
        self.lineEdit = QtGui.QLineEdit()
        self.setButton = QtGui.QPushButton("Set")
        self.setButton.clicked.connect(self.setter)

        self.lockCheckBox = QtGui.QCheckBox("Lock")
        self.lockCheckBox.stateChanged.connect(self.lock)

        self.modeRadioGrp = QtGui.QButtonGroup()
        self.vertexMode = QtGui.QRadioButton('Vertex')
        self.normalMode = QtGui.QRadioButton('Normal')
        self.surfaceMode = QtGui.QRadioButton('Surface')
        self.normalMode.setChecked(True)

        self.modeRadioGrp.addButton(self.vertexMode)
        self.modeRadioGrp.addButton(self.normalMode)
        self.modeRadioGrp.addButton(self.surfaceMode)
        self.modeRadioGrp.setId(self.vertexMode, 1)
        self.modeRadioGrp.setId(self.normalMode, 2)
        self.modeRadioGrp.setId(self.surfaceMode, 3)

        self.distanceLE = QtGui.QLineEdit("99999")
        self.distanceLock = QtGui.QCheckBox("Lock")
        self.distanceLock.stateChanged.connect(self.lockDistance)

        self.snapButton = QtGui.QPushButton("Snap")
        self.snapButton.setFixedHeight(40)
        self.snapButton.clicked.connect(self.snap)

    def layoutUI(self):
        topLayout = QtGui.QBoxLayout(QtGui.QBoxLayout.LeftToRight)
        topLayout.addWidget(QtGui.QLabel("Snap Target : "))
        topLayout.addWidget(self.lineEdit)
        topLayout.addWidget(self.setButton)
        topLayout.addWidget(self.lockCheckBox)

        modeLayout = QtGui.QBoxLayout(QtGui.QBoxLayout.LeftToRight)
        modeLayout.addWidget(QtGui.QLabel("Snap Mode : "))
        modeLayout.addWidget(self.vertexMode)
        modeLayout.addWidget(self.normalMode)
        modeLayout.addWidget(self.surfaceMode)

        distLayout = QtGui.QBoxLayout(QtGui.QBoxLayout.LeftToRight)
        distLayout.addWidget(QtGui.QLabel("Snap Max Distance : "))
        distLayout.addWidget(self.distanceLE)
        distLayout.addWidget(self.distanceLock)

        mainLayout = QtGui.QBoxLayout(QtGui.QBoxLayout.TopToBottom)
        mainLayout.addLayout(topLayout)
        mainLayout.addLayout(modeLayout)
        mainLayout.addLayout(distLayout)
        mainLayout.addWidget(self.snapButton)

        self.setLayout(mainLayout)

    def lock(self):
        if self.lockCheckBox.checkState() == QtCore.Qt.CheckState.Checked:
            self.lineEdit.setEnabled(False)
        else:
            self.lineEdit.setEnabled(True)

    def lockDistance(self):
        if self.distanceLock.checkState() == QtCore.Qt.CheckState.Checked:
            self.distanceLE.setEnabled(False)
        else:
            self.distanceLE.setEnabled(True)

    def snap(self):
        target = self.lineEdit.text()
        if target == "":
            return

        check = self.modeRadioGrp.checkedId()
        if check == 1:
            snapMode = "vertex"
        elif check == 2:
            snapMode = "normal"
        else:
            snapMode = "surface"

        try:
            maxDist = int(self.distanceLE.text())
        except:
            print "not int!!!!"
            return

        cmds.snapToClosest(
            target,
            mode=snapMode,
            d=maxDist)

    def setter(self):
        self.lineEdit.setText(cmds.ls(sl=True, fl=True, long=True)[0])


if __name__ == "__main__":
    snap = SnapWindow()
    snap.show()
    snap.raise_()
