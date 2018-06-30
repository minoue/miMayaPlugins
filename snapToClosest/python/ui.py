from Qt import QtWidgets, QtCore
from maya import OpenMayaUI
from maya import cmds
try:
    import shiboken
except ImportError:
    import shiboken2 as shiboken


def getMayaWindow():
    ptr = OpenMayaUI.MQtUtil.mainWindow()
    return shiboken.wrapInstance(long(ptr), QtWidgets.QMainWindow)


class SnapWindow(QtWidgets.QDialog):

    def closeExistingWindow(self):
        """ Close window if exists """

        for qt in QtWidgets.QApplication.topLevelWidgets():
            try:
                if qt.__class__.__name__ == self.__class__.__name__:
                    qt.close()
            except:
                pass

    def __init__(self, parent=getMayaWindow()):
        self.closeExistingWindow()

        print parent
        super(SnapWindow, self).__init__(parent)

        self.setWindowTitle("Snap")
        self.setWindowFlags(QtCore.Qt.Window)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)

        self.setFixedWidth(600)

        self.createUI()
        self.layoutUI()

    def createUI(self):
        self.lineEdit = QtWidgets.QLineEdit()
        self.lineEdit.setEnabled(False)
        self.setButton = QtWidgets.QPushButton("Set")
        self.setButton.clicked.connect(self.setter)

        self.modeRadioGrp = QtWidgets.QButtonGroup()
        self.vertexMode = QtWidgets.QRadioButton('Vertex')
        self.normalMode = QtWidgets.QRadioButton('Normal')
        self.surfaceMode = QtWidgets.QRadioButton('Surface')
        self.normalMode.setChecked(True)

        self.modeRadioGrp.addButton(self.vertexMode)
        self.modeRadioGrp.addButton(self.normalMode)
        self.modeRadioGrp.addButton(self.surfaceMode)
        self.modeRadioGrp.setId(self.vertexMode, 1)
        self.modeRadioGrp.setId(self.normalMode, 2)
        self.modeRadioGrp.setId(self.surfaceMode, 3)

        self.distanceLE = QtWidgets.QLineEdit("99999")
        self.distanceLock = QtWidgets.QCheckBox("Lock")
        self.distanceLock.stateChanged.connect(self.lockDistance)

        self.customVectorCheck = QtWidgets.QCheckBox("Use custom vector")
        self.customVectorCheck.stateChanged.connect(
            self.changeCustomVectorState)
        self.customVectorX = QtWidgets.QLineEdit("0")
        self.customVectorY = QtWidgets.QLineEdit("0")
        self.customVectorZ = QtWidgets.QLineEdit("0")
        self.customVectorX.setEnabled(False)
        self.customVectorY.setEnabled(False)
        self.customVectorZ.setEnabled(False)
        self.tbdCheckBox = QtWidgets.QCheckBox("Test both directions")

        self.snapButton = QtWidgets.QPushButton("Snap")
        self.snapButton.setFixedHeight(40)
        self.snapButton.clicked.connect(self.snap)

    def layoutUI(self):
        topLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.LeftToRight)
        topLayout.addWidget(QtWidgets.QLabel("Snap Target : "))
        topLayout.addWidget(self.lineEdit)
        topLayout.addWidget(self.setButton)

        modeLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.LeftToRight)
        modeLayout.addWidget(QtWidgets.QLabel("Snap Mode : "))
        modeLayout.addWidget(self.vertexMode)
        modeLayout.addWidget(self.normalMode)
        modeLayout.addWidget(self.surfaceMode)

        distLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.LeftToRight)
        distLayout.addWidget(QtWidgets.QLabel("Snap Max Distance : "))
        distLayout.addWidget(self.distanceLE)
        distLayout.addWidget(self.distanceLock)

        cvLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.LeftToRight)
        cvLayout.addWidget(self.customVectorCheck)
        cvLayout.addWidget(self.customVectorX)
        cvLayout.addWidget(self.customVectorY)
        cvLayout.addWidget(self.customVectorZ)
        cvLayout.addWidget(self.tbdCheckBox)

        mainLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.TopToBottom)
        mainLayout.addLayout(topLayout)
        mainLayout.addLayout(modeLayout)
        mainLayout.addLayout(distLayout)
        mainLayout.addLayout(cvLayout)
        mainLayout.addWidget(self.snapButton)

        self.setLayout(mainLayout)

    def changeCustomVectorState(self):
        if self.customVectorCheck.checkState() == QtCore.Qt.CheckState.Checked:
            self.customVectorX.setEnabled(True)
            self.customVectorY.setEnabled(True)
            self.customVectorZ.setEnabled(True)
        else:
            self.customVectorX.setEnabled(False)
            self.customVectorY.setEnabled(False)
            self.customVectorZ.setEnabled(False)

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

        # directions
        if self.tbdCheckBox.checkState() == QtCore.Qt.CheckState.Checked:
            testBothDirections = True
        else:
            testBothDirections = False

        try:
            maxDist = int(self.distanceLE.text())
        except:
            print "not int!!!!"
            return

        if self.customVectorCheck.checkState() == QtCore.Qt.CheckState.Checked:
            nx = float(self.customVectorX.text())
            ny = float(self.customVectorY.text())
            nz = float(self.customVectorZ.text())
            cmds.snapToClosest(
                target,
                mode=snapMode,
                d=maxDist,
                cv=True,
                cvx=nx,
                cvy=ny,
                cvz=nz,
                tbd=testBothDirections)
        else:
            cmds.snapToClosest(
                target,
                mode=snapMode,
                d=maxDist,
                tbd=testBothDirections)

    def setter(self):
        self.lineEdit.setText(cmds.ls(sl=True, fl=True, long=True)[0])


def main():
    snap = SnapWindow()
    snap.show()
    snap.raise_()


if __name__ == "__main__":
    main()
