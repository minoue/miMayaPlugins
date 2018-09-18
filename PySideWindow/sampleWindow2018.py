from maya.app.general.mayaMixin import MayaQWidgetDockableMixin
from PySide2 import QtCore, QtWidgets
from maya import OpenMayaUI
from maya import cmds
import shiboken2


def mayaUIContent(parent):
    """ Contents by Maya standard UI widgets """

    layout = cmds.columnLayout(adjustableColumn=True, parent=parent)

    cmds.frameLayout("Sample Frame 1", collapsable=True)
    cmds.columnLayout(adjustableColumn=True, rowSpacing=2)
    cmds.button("maya button 1")
    cmds.button("maya button 2")
    cmds.button("maya button 3")
    cmds.setParent('..')
    cmds.setParent('..')

    cmds.frameLayout("Sample Frame 2", collapsable=True)
    cmds.gridLayout(numberOfColumns=6, cellWidthHeight=(35, 35))
    cmds.shelfButton(image1="polySphere.png", rpt=True, c=cmds.polySphere)
    cmds.shelfButton(image1="sphere.png", rpt=True, c=cmds.sphere)
    cmds.setParent('..')
    cmds.setParent('..')

    cmds.setParent('..')  # columnLayout

    ptr = OpenMayaUI.MQtUtil.findLayout(layout)
    obj = shiboken2.wrapInstance(long(ptr), QtWidgets.QWidget)

    return obj


class Content(QtWidgets.QWidget):
    """ Contents widget for tabs """

    def __init__(self, parent=None):
        """ Init """

        super(Content, self).__init__(parent)

        self.button1 = QtWidgets.QPushButton("button1")
        self.button2 = QtWidgets.QPushButton("button2")
        self.le = QtWidgets.QLineEdit("lineedit")
        self.textEdit = QtWidgets.QTextEdit("text edit")

        layout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.TopToBottom)
        layout.addWidget(self.button1)
        layout.addWidget(self.button2)
        layout.addWidget(self.le)
        layout.addWidget(self.textEdit)

        self.setLayout(layout)


class CentralWidget(QtWidgets.QWidget):
    """ Central widget """

    def __init__(self, parent=None):
        """ Init """

        super(CentralWidget, self).__init__(parent)

        self.createUI()
        self.layoutUI()

    def createUI(self):
        """ Crete widgets """

        self.tabWidget = QtWidgets.QTabWidget()
        self.tabWidget.addTab(Content(self), "Tab1")
        self.tabWidget.addTab(Content(self), "Tab2")

    def layoutUI(self):
        """ Layout widgets """

        mainLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.TopToBottom)
        mainLayout.addWidget(self.tabWidget)

        self.setLayout(mainLayout)


class MainWindow(MayaQWidgetDockableMixin, QtWidgets.QMainWindow):

    def __init__(self, parent=None):
        """ init """

        super(MainWindow, self).__init__(parent)

        self.thisObjectName = "testDockWindow"
        self.WindowTitle = "Sample Dockable Widget"
        self.workspaceControlName = self.thisObjectName + "WorkspaceControl"

        self.setObjectName(self.thisObjectName)
        self.setWindowTitle(self.WindowTitle)

        self.setWindowFlags(QtCore.Qt.Window)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)

        # Create and set central widget
        self.cw = CentralWidget()
        self.setCentralWidget(self.cw)

        self.setupMenu()

    def setupMenu(self):
        """ Setup menu """

        menu = self.menuBar()

        # About
        aboutAction = QtWidgets.QAction("&About", self)
        aboutAction.setStatusTip('About this script')
        aboutAction.triggered.connect(self.showAbout)

        menu.addAction("File")
        help_menu = menu.addMenu("&Help")
        help_menu.addAction(aboutAction)

    def showAbout(self):
        """ about message """

        QtWidgets.QMessageBox.about(
            self,
            'About ',
            'Awesome window\n')

    def run(self):
        try:
            cmds.deleteUI(self.workspaceControlName)
        except RuntimeError:
            pass

        self.show(dockable=True)
        cmds.workspaceControl(
            self.workspaceControlName,
            edit=True,
            dockToControl=['Outliner', 'right'])
        self.raise_()

        # Maya layout widget is added here to be parented under workspaceControl
        self.cw.tabWidget.addTab(mayaUIContent(self.workspaceControlName), "MayaLayout")


def main():
    w = MainWindow()
    w.run()


if __name__ == "__main__":
    main()
