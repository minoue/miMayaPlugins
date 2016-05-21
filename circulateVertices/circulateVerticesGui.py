#
#  circulateVerticesGui.py
#
#
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <michitaka_inoue@icloud.com> wrote this file.  As long as you retain this
# notice you can do whatever you want with this stuff. If we meet some day,
# and you think this stuff is worth it, you can buy me a beer in return.
# -Michitaka Inoue
# ----------------------------------------------------------------------------
#


from maya import OpenMayaUI
from maya import cmds
from PySide import QtGui, QtCore
import shiboken


def getMayaWindow():
    """ Get maya main window object.
    """
    ptr = OpenMayaUI.MQtUtil.mainWindow()
    return shiboken.wrapInstance(long(ptr), QtGui.QMainWindow)


class CirculateVerticesGui(QtGui.QWidget):

    def closeExistingWindow(self):
        for qt in QtGui.QApplication.topLevelWidgets():
            try:
                if qt.__class__.__name__ == self.__class__.__name__:
                    qt.close()
            except:
                pass

    def __init__(self, parent=getMayaWindow()):
        self.closeExistingWindow()
        super(CirculateVerticesGui, self).__init__(parent)

        self.setWindowTitle("Circulate Vertices")
        self.setWindowFlags(QtCore.Qt.Window)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        self.resize(0, 0)

        self.createUI()
        self.layoutUI()

    def createUI(self):
        self.button = QtGui.QPushButton("Circulate")
        self.button.setFixedHeight(30)
        self.button.clicked.connect(self.run)
        self.invertCheckBox = QtGui.QCheckBox()
        self.multiplyLE = QtGui.QLineEdit("1.0")
        self.rotateLE = QtGui.QLineEdit("0.0")

    def layoutUI(self):
        form = QtGui.QFormLayout()
        form.addRow("Invert Direction", self.invertCheckBox)
        form.addRow("Multiply", self.multiplyLE)
        form.addRow("Rotate", self.rotateLE)

        layout = QtGui.QBoxLayout(QtGui.QBoxLayout.TopToBottom)
        layout.addLayout(form)
        layout.addWidget(self.button)

        self.setLayout(layout)

    def run(self):
        r = float(self.rotateLE.text())
        m = float(self.multiplyLE.text())
        if self.invertCheckBox.isChecked():
            cmds.circulateVertices(i=True, rotate=r, multiply=m)
        else:
            cmds.circulateVertices(i=False, rotate=r, multiply=m)


def main():

    global cw
    try:
        cw.close()
    except:
        pass
    cw = CirculateVerticesGui()
    cw.show()


if __name__ == "__main__":
    main()
