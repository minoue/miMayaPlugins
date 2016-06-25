from PySide import QtGui, QtCore
from maya import OpenMayaUI
import shiboken


def getMayaWindow():
    ptr = OpenMayaUI.MQtUtil.mainWindow()
    return shiboken.wrapInstance(long(ptr), QtGui.QMainWindow)


class Content(QtGui.QWidget):
    """ Contents widget for tabs """

    def __init__(self, parent=None):
        """ Init """

        super(Content, self).__init__(parent)

        self.button = QtGui.QPushButton("button")
        self.le = QtGui.QLineEdit("lineedit")
        self.textEdit = QtGui.QTextEdit("text edit")

        layout = QtGui.QBoxLayout(QtGui.QBoxLayout.TopToBottom)
        layout.addWidget(self.button)
        layout.addWidget(self.le)
        layout.addWidget(self.textEdit)

        self.setLayout(layout)


class CentralWidget(QtGui.QWidget):
    """ Central widget """

    def __init__(self, parent=None):
        """ Init """

        super(CentralWidget, self).__init__(parent)

        self.createUI()
        self.layoutUI()

    def createUI(self):
        """ Crete widgets """

        self.tabWidget = QtGui.QTabWidget()
        self.tabWidget.addTab(Content(), "Tab1")
        self.tabWidget.addTab(Content(), "Tab2")

    def layoutUI(self):
        """ Layout widgets """

        mainLayout = QtGui.QBoxLayout(QtGui.QBoxLayout.TopToBottom)
        mainLayout.addWidget(self.tabWidget)

        self.setLayout(mainLayout)


class MainWindow(QtGui.QMainWindow):
    """ Main window """

    def closeExistingWindow(self):
        """ Close window if exists """

        for qt in QtGui.QApplication.topLevelWidgets():
            try:
                if qt.__class__.__name__ == self.__class__.__name__:
                    qt.close()
            except:
                pass

    def __init__(self, parent=getMayaWindow()):
        """ init """

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


def mainWindow(func):
    """ Decorate main/dock fuctions """

    def __wrapper(*args):

        """ close window if exist """
        global win
        try:
            win.close()
        except:
            pass

        # New main window object
        win = MainWindow()
        func(win)

    return __wrapper


@mainWindow
def main(mainWindow):
    """ Show single window
        args
            mainWindow : QtGui.QMainWindow
        return
            None
    """

    mainWindow.show()
    mainWindow.raise_()


@mainWindow
def dock(mainWindow):
    """ Show dockable window
        args
            mainWindow : QtGui.QMainWindow
        return
            None
    """

    mainWindow.setObjectName('sampleWindowObject')

    DOCK_NAME = "dock_name"

    from pymel import all as pm

    if pm.dockControl(DOCK_NAME, q=True, ex=1):
        pm.deleteUI(DOCK_NAME)

    if pm.window('dummyWindow', q=True, ex=1):
        pm.deleteUI('dummyWindow')

    # Create dummy window object to keep the layout
    pm.window('dummyWindow')

    pm.columnLayout()
    floatingLayout = pm.paneLayout(
        configuration='single',
        w=300)
    pm.setParent('..')

    # Create new dock
    pm.dockControl(
        DOCK_NAME,
        aa=['right', 'left'],
        a='right',
        fl=False,
        con=floatingLayout,
        label="Sample Dock",
        w=300)

    # Parent QMainWindow object to the layout
    pm.control('sampleWindowObject', e=True, parent=floatingLayout)


if __name__ == "__main__":
    # main()
    dock()
