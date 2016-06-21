from PySide import QtGui, QtCore
from maya import OpenMayaUI
import shiboken


def getMayaWindow():
    ptr = OpenMayaUI.MQtUtil.mainWindow()
    return shiboken.wrapInstance(long(ptr), QtGui.QMainWindow)


class Content(QtGui.QWidget):

    def __init__(self, parent=None):
        super(Content, self).__init__(parent)

        self.button = QtGui.QPushButton("button")
        self.le = QtGui.QLineEdit("lineedit")

        layout = QtGui.QBoxLayout(QtGui.QBoxLayout.TopToBottom)
        layout.addWidget(self.button)
        layout.addWidget(self.le)

        self.setLayout(layout)


class CentralWidget(QtGui.QWidget):

    def __init__(self, parent=None):
        super(CentralWidget, self).__init__(parent)

        self.createUI()
        self.layoutUI()

    def createUI(self):

        self.tabWidget = QtGui.QTabWidget()
        self.tabWidget.addTab(Content(), "Tab1")
        self.tabWidget.addTab(Content(), "Tab2")

    def layoutUI(self):
        mainLayout = QtGui.QBoxLayout(QtGui.QBoxLayout.TopToBottom)
        mainLayout.addWidget(self.tabWidget)

        self.setLayout(mainLayout)


class MainWindow(QtGui.QMainWindow):

    def closeExistingWindow(self):
        for qt in QtGui.QApplication.topLevelWidgets():
            try:
                if qt.__class__.__name__ == self.__class__.__name__:
                    qt.close()
            except:
                pass

    def __init__(self, parent=getMayaWindow()):
        self.closeExistingWindow()
        super(MainWindow, self).__init__(parent)

        self.setWindowFlags(QtCore.Qt.Window)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)

        # Create and set central widget
        cw = CentralWidget()
        self.setCentralWidget(cw)

        self.setupMenu()

    def setupMenu(self):
        """ Setup menu """
        menu = self.menuBar()

        # About
        aboutAction = QtGui.QAction("&About", self)
        aboutAction.setStatusTip('About this script')
        aboutAction.triggered.connect(self.showAbout)

        menu.addAction("File")
        help_menu = menu.addMenu("&Help")
        help_menu.addAction(aboutAction)

    def showAbout(self):
        """ about message """

        QtGui.QMessageBox.about(
            self,
            'About ',
            'Awesome window\n')


def main():
    """ main """

    global win
    try:
        win.close()
    except:
        pass

    win = MainWindow()
    win.show()
    win.raise_()


def dock():
    """ dock """

    global win
    try:
        win.close()
    except:
        pass

    win = MainWindow()
    win.setObjectName('sampleWindowObject')

    DOCK_NAME = "dock_name"

    from pymel import all as pm
    if pm.dockControl(DOCK_NAME, q=True, ex=1):
        pm.deleteUI(DOCK_NAME)
    floatingLayout = pm.paneLayout(configuration='single', w=300)

    pm.dockControl(
        DOCK_NAME,
        aa=['right', 'left'],
        a='right',
        fl=False,
        con=floatingLayout,
        label="Sample Dock",
        w=300)

    pm.control('sampleWindowObject', e=True, parent=floatingLayout)


if __name__ == "__main__":
    # main()
    dock()
