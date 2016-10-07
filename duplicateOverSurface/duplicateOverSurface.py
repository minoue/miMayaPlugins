#
#  duplicaeOnSurface.py
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

from maya import OpenMaya
from maya import OpenMayaUI
from maya import OpenMayaMPx
from maya import cmds
try:
    from PySide.QtGui import QApplication
    from PySide import QtCore
except ImportError:
    from PySide2.QtWidgets import QApplication
    from PySide2 import QtCore
import math
import sys


DRAGGER = "duplicateOverSurfaceDragger"
UTIL = OpenMaya.MScriptUtil()


kPluginCmdName = "duplicateOverSurface"
kRotationFlag = "-r"
kRotationFlagLong = "-rotation"
kDummyFlag = "-d"
kDummyFlagLong = "-dummy"
kInstanceFlag = "-ilf"
kInstanceFlagLong = "-instanceLeaf"


# Syntax creator
def syntaxCreator():
    syntax = OpenMaya.MSyntax()
    syntax.addArg(OpenMaya.MSyntax.kString)
    syntax.addFlag(
        kDummyFlag,
        kDummyFlagLong,
        OpenMaya.MSyntax.kBoolean)
    syntax.addFlag(
        kRotationFlag,
        kRotationFlagLong,
        OpenMaya.MSyntax.kBoolean)
    syntax.addFlag(
        kInstanceFlag,
        kInstanceFlagLong,
        OpenMaya.MSyntax.kBoolean)
    return syntax


class DuplicateOverSurface(OpenMayaMPx.MPxCommand):

    def __init__(self):
        super(DuplicateOverSurface, self).__init__()

        self.ANCHOR_POINT = None
        self.DUPLICATED = None
        self.SOURCE = None
        self.SCALE_ORIG = None
        self.MATRIX_ORIG = None
        self.TARGET_FNMESH = None
        self.MOD_FIRST = None
        self.MOD_POINT = None
        self.SPACE = OpenMaya.MSpace.kWorld

        self.ROTATION = True
        self.InstanceFlag = False

        self.SHIFT = QtCore.Qt.ShiftModifier
        self.CTRL = QtCore.Qt.ControlModifier

    def doIt(self, args):

        # Parse the arguments.
        argData = OpenMaya.MArgDatabase(syntaxCreator(), args)
        self.SOURCE = argData.commandArgumentString(0)
        if argData.isFlagSet(kRotationFlag) is True:
            self.ROTATION = argData.flagArgumentBool(kRotationFlag, 0)

        if argData.isFlagSet(kInstanceFlag) is True:
            self.InstanceFlag = argData.flagArgumentBool(kInstanceFlag, 0)

        cmds.setToolTo(self.setupDragger())

    def setupDragger(self):
        """ Setup dragger context command """

        try:
            cmds.deleteUI(DRAGGER)
        except:
            pass

        dragger = cmds.draggerContext(
            DRAGGER,
            pressCommand=self.pressEvent,
            dragCommand=self.dragEvent,
            releaseCommand=self.releaseEvent,
            space='screen',
            projection='viewPlane',
            undoMode='step',
            cursor='hand')

        return dragger

    def pressEvent(self):
        button = cmds.draggerContext(DRAGGER, query=True, button=True)

        # Leave the tool by middle click
        if button == 2:
            cmds.setToolTo('selectSuperContext')
            return

        # Get clicked point in viewport screen space
        pressPosition = cmds.draggerContext(DRAGGER, query=True, ap=True)
        x = pressPosition[0]
        y = pressPosition[1]

        self.ANCHOR_POINT = [x, y]

        # Convert
        point_in_3d, vector_in_3d = convertTo3D(x, y)

        # Get MFnMesh of snap target
        targetDagPath = getDagPathFromScreen(x, y)

        # If draggin outside of objects
        if targetDagPath is None:
            return

        # Get origianl scale information
        self.SCALE_ORIG = cmds.getAttr(self.SOURCE + ".scale")[0]
        self.MATRIX_ORIG = cmds.xform(self.SOURCE, q=True, matrix=True)
        self.TARGET_FNMESH = OpenMaya.MFnMesh(targetDagPath)

        transformMatrix = self.getMatrix(
            point_in_3d,
            vector_in_3d,
            self.TARGET_FNMESH,
            self.SCALE_ORIG,
            self.MATRIX_ORIG)

        if transformMatrix is None:
            return

        # Create new object to snap
        self.DUPLICATED = self.getNewObject()

        # Reset transform of current object
        cmds.setAttr(self.DUPLICATED + ".translate", *[0, 0, 0])

        location = [-i for i
                    in cmds.xform(self.DUPLICATED, q=True, ws=True, rp=True)]
        cmds.setAttr(self.DUPLICATED + ".translate", *location)

        # Can't apply freeze to instances
        if self.InstanceFlag is not True:
            cmds.makeIdentity(self.DUPLICATED, apply=True, t=True)

        # Apply transformMatrix to the new object
        cmds.xform(self.DUPLICATED, matrix=transformMatrix)

    def getNewObject(self):
        return cmds.duplicate(self.SOURCE, ilf=self.InstanceFlag)[0]

    def dragEvent(self):
        """ Event while dragging a 3d view """

        if self.TARGET_FNMESH is None:
            return

        dragPosition = cmds.draggerContext(
            DRAGGER,
            query=True,
            dragPoint=True)

        x = dragPosition[0]
        y = dragPosition[1]

        modifier = cmds.draggerContext(
            DRAGGER,
            query=True,
            modifier=True)

        if modifier == "none":
            self.MOD_FIRST = True

        qtModifier = QApplication.keyboardModifiers()

        if qtModifier == self.CTRL or qtModifier == self.SHIFT:

            # If this is the first click of dragging
            if self.MOD_FIRST is True:
                self.MOD_POINT = [x, y]

                # global MOD_FIRST
                self.MOD_FIRST = False

            length, degree = self.getDragInfo(x, y)

            if qtModifier == self.CTRL:
                length = 1.0
            if qtModifier == self.SHIFT:
                degree = 0.0

            # Convert
            point_in_3d, vector_in_3d = convertTo3D(
                self.MOD_POINT[0],
                self.MOD_POINT[1])
        else:
            point_in_3d, vector_in_3d = convertTo3D(x, y)
            length = 1.0
            degree = 0.0

        # Get new transform matrix for new object
        transformMatrix = self.getMatrix(
            point_in_3d,
            vector_in_3d,
            self.TARGET_FNMESH,
            self.SCALE_ORIG,
            self.MATRIX_ORIG,
            length,
            degree
            )

        if transformMatrix is None:
            return

        # Apply new transform
        cmds.xform(self.DUPLICATED, matrix=transformMatrix)
        cmds.setAttr(self.DUPLICATED + ".shear", *[0, 0, 0])

        cmds.refresh(currentView=True, force=True)

    def releaseEvent(self):
        self.MOD_FIRST = True

    def getDragInfo(self, x, y):
        """ Get distance and angle in screen space. """

        start_x = self.MOD_POINT[0]
        start_y = self.MOD_POINT[1]
        end_x = x
        end_y = y

        cathetus = end_x - start_x
        opposite = end_y - start_y

        # Get distance using Pythagorean theorem
        length = math.sqrt(
            math.pow(cathetus, 2) + math.pow(opposite, 2))

        try:
            theta = cathetus / length
            degree = math.degrees(math.acos(theta))
            if opposite < 0:
                degree = -degree
            return cathetus, degree
        except ZeroDivisionError:
            return None, None

    def getIntersection(self, point_in_3d, vector_in_3d, fnMesh):
        """ Return a point Position of intersection..
            Args:
                point_in_3d  (OpenMaya.MPoint)
                vector_in_3d (OpenMaya.mVector)
            Returns:
                OpenMaya.MFloatPoint : hitPoint
        """

        hitPoint = OpenMaya.MFloatPoint()
        hitFacePtr = UTIL.asIntPtr()
        idSorted = False
        testBothDirections = False
        faceIDs = None
        triIDs = None
        accelParam = None
        hitRayParam = None
        hitTriangle = None
        hitBary1 = None
        hitBary2 = None
        maxParamPtr = 99999

        # intersectPoint = OpenMaya.MFloatPoint(
        result = fnMesh.closestIntersection(
            OpenMaya.MFloatPoint(
                point_in_3d.x,
                point_in_3d.y,
                point_in_3d.z),
            OpenMaya.MFloatVector(vector_in_3d),
            faceIDs,
            triIDs,
            idSorted,
            self.SPACE,
            maxParamPtr,
            testBothDirections,
            accelParam,
            hitPoint,
            hitRayParam,
            hitFacePtr,
            hitTriangle,
            hitBary1,
            hitBary2)

        faceID = UTIL.getInt(hitFacePtr)

        if result is True:
            return hitPoint, faceID
        else:
            return None, None

    def getMatrix(self,
                  mPoint,
                  mVector,
                  targetFnMesh,
                  scale_orig,
                  matrix_orig,
                  scale_plus=1,
                  degree_plus=0.0):

        """ Return a list of values which consist a new transform matrix.
            Args:
                mPoint  (OpenMaya.MPoint)
                mVector (OpenMaya.MVector)
            Returns:
                list : 16 values for matrixs
        """
        # Position of new object
        OP, faceID = self.getIntersection(mPoint, mVector, targetFnMesh)

        # If it doesn't intersect to any geometries, return None
        if OP is None and faceID is None:
            return None

        qtMod = QApplication.keyboardModifiers()
        if qtMod == (self.CTRL | self.SHIFT):
            OP = getClosestVertex(OP, faceID, targetFnMesh)

        # Get normal vector and tangent vector
        if self.ROTATION is False:
            NV = OpenMaya.MVector(
                matrix_orig[4],
                matrix_orig[5],
                matrix_orig[6])
            NV.normalize()
            TV = OpenMaya.MVector(
                matrix_orig[0],
                matrix_orig[1],
                matrix_orig[2])
            TV.normalize()
        else:
            NV = self.getNormal(OP, targetFnMesh)
            TV = self.getTangent(faceID, targetFnMesh)

        # Ctrl-hold rotation
        if qtMod == self.CTRL:
            try:
                rad = math.radians(degree_plus)
                q1 = NV.x * math.sin(rad / 2)
                q2 = NV.y * math.sin(rad / 2)
                q3 = NV.z * math.sin(rad / 2)
                q4 = math.cos(rad / 2)
                TV = TV.rotateBy(q1, q2, q3, q4)
            except TypeError:
                pass

        # Bitangent vector
        BV = TV ^ NV
        BV.normalize()

        # 4x4 Transform Matrix
        try:
            x = scale_orig[0] * (scale_plus / 100 + 1.0)
            y = scale_orig[1] * (scale_plus / 100 + 1.0)
            z = scale_orig[2] * (scale_plus / 100 + 1.0)
            TV *= x
            NV *= y
            BV *= z
        except TypeError:
            pass
        finally:
            matrix = [
                TV.x, TV.y, TV.z, 0,
                NV.x, NV.y, NV.z, 0,
                BV.x, BV.y, BV.z, 0,
                OP.x, OP.y, OP.z, 1
            ]

        return matrix

    def getTangent(self, faceID, targetFnMesh):
        """ Return a tangent vector of a face.
            Args:
                faceID  (int)
                mVector (OpenMaya.MVector)
            Returns:
                OpenMaya.MVector : tangent vector
        """

        tangentArray = OpenMaya.MFloatVectorArray()
        targetFnMesh.getFaceVertexTangents(
            faceID,
            tangentArray,
            self.SPACE)
        numOfVtx = tangentArray.length()
        x = sum([tangentArray[i].x for i in range(numOfVtx)]) / numOfVtx
        y = sum([tangentArray[i].y for i in range(numOfVtx)]) / numOfVtx
        z = sum([tangentArray[i].z for i in range(numOfVtx)]) / numOfVtx
        tangentVector = OpenMaya.MVector()
        tangentVector.x = x
        tangentVector.y = y
        tangentVector.z = z
        tangentVector.normalize()

        return tangentVector

    def getNormal(self, pointPosition, targetFnMesh):
        """ Return a normal vector of a face.
            Args:
                pointPosition  (OpenMaya.MFloatPoint)
                targetFnMesh (OpenMaya.MFnMesh)
            Returns:
                OpenMaya.MVector : tangent vector
                int              : faceID
        """

        ptr_int = UTIL.asIntPtr()
        origin = OpenMaya.MPoint(pointPosition)
        normal = OpenMaya.MVector()
        targetFnMesh.getClosestNormal(
            origin,
            normal,
            self.SPACE,
            ptr_int)
        normal.normalize()

        return normal


# Creator
def cmdCreator():
    return OpenMayaMPx.asMPxPtr(DuplicateOverSurface())


def initializePlugin(mObject):
    mPlugin = OpenMayaMPx.MFnPlugin(mObject, "Michitaka Inoue")
    try:
        mPlugin.registerCommand(kPluginCmdName, cmdCreator)
        mPlugin.setVersion("0.10")
    except:
        sys.stderr.write("Failed to register command: %s\n" % kPluginCmdName)
        raise


def uninitializePlugin(mObject):
    mPlugin = OpenMayaMPx.MFnPlugin(mObject)
    try:
        mPlugin.deregisterCommand(kPluginCmdName)
    except:
        sys.stderr.write("Failed to unregister command: %s\n" % kPluginCmdName)


def convertTo3D(screen_x, screen_y):
    """ Return point and vector of clicked point in 3d space.
        Args:
            screen_x  (int)
            screen_y (int)
        Returns:
            OpenMaya.MPoint : point_in_3d
            OpenMaya.MVector : vector_in_3d
    """
    point_in_3d = OpenMaya.MPoint()
    vector_in_3d = OpenMaya.MVector()

    OpenMayaUI.M3dView.active3dView().viewToWorld(
        int(screen_x),
        int(screen_y),
        point_in_3d,
        vector_in_3d)

    return point_in_3d, vector_in_3d


def getDagPathFromScreen(x, y):
    """ Args:
            x  (int or float)
            y (int or float)
        Returns:
            dagpath : OpenMaya.MDagPath
    """
    # Select from screen
    OpenMaya.MGlobal.selectFromScreen(
        int(x),
        int(y),
        OpenMaya.MGlobal.kReplaceList,
        OpenMaya.MGlobal.kSurfaceSelectMethod)

    # Get dagpath, or return None if fails
    tempSel = OpenMaya.MSelectionList()
    OpenMaya.MGlobal.getActiveSelectionList(tempSel)
    dagpath = OpenMaya.MDagPath()
    if tempSel.length() == 0:
        return None
    else:
        tempSel.getDagPath(0, dagpath)
        return dagpath


def getClosestVertex(point_orig, faceID, fnMesh):
    """ Args:
            point_orig  (OpenMaya.MFloatPoint)
            faceID (int)
            fnMesh (OpenMaya.MFnMesh)
        Returns:
            closestPoint : OpenMaya.MPoint
    """

    vertexIndexArray = OpenMaya.MIntArray()
    fnMesh.getPolygonVertices(faceID, vertexIndexArray)
    basePoint = OpenMaya.MPoint(point_orig)
    closestPoint = OpenMaya.MPoint()
    length = 99999.0
    for index in vertexIndexArray:
        point = OpenMaya.MPoint()
        fnMesh.getPoint(index, point, OpenMaya.MSpace.kWorld)
        lengthVector = point - basePoint
        if lengthVector.length() < length:
            length = lengthVector.length()
            closestPoint = point

    return closestPoint
