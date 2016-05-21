#
#  circulateVertices.py
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

from pymel import all as pm
from maya import OpenMaya
from maya import OpenMayaMPx
from maya import cmds
import math
import sys


kPluginCmdName = "circulateVertices"
kMultiplyFlag = "-m"
kMultiplyFlagLong = "-multiply"
kRotateFlag = "-r"
kRotateFlagLong = "-rotate"
kInvertedDirectionFlag = "-i"
kInvertedDirectionFlagLong = "-invertedDirection"


def syntaxCreator():
    syntax = OpenMaya.MSyntax()

    syntax.addFlag(
        kMultiplyFlag,
        kMultiplyFlagLong,
        OpenMaya.MSyntax.kDouble)

    syntax.addFlag(
        kRotateFlag,
        kRotateFlagLong,
        OpenMaya.MSyntax.kDouble)

    syntax.addFlag(
        kInvertedDirectionFlag,
        kInvertedDirectionFlagLong,
        OpenMaya.MSyntax.kBoolean)

    return syntax


class CirculateVertices(OpenMayaMPx.MPxCommand):

    def __init__(self):
        super(CirculateVertices, self).__init__()

        self.UTIL = OpenMaya.MScriptUtil()
        self.MULTIPLY = 1.0
        self.INVERTED = False
        self.ROTATION = 0.0
        self.CENTER = OpenMaya.MPoint()

    def isUndoable(self):
        return True

    def doIt(self, args):
        """ Just do it! """

        # Parse the arguments.
        argData = OpenMaya.MArgDatabase(syntaxCreator(), args)

        if argData.isFlagSet(kMultiplyFlag):
            self.MULTIPLY = argData.flagArgumentDouble(kMultiplyFlag, 0)

        if argData.isFlagSet(kInvertedDirectionFlag):
            self.INVERTED = argData.flagArgumentBool(kInvertedDirectionFlag, 0)

        if argData.isFlagSet(kRotateFlag):
            self.ROTATION = argData.flagArgumentDouble(kRotateFlag, 0)

        self.redoIt()

    def redoIt(self):
        """ redo it """

        cmds.undoInfo(openChunk=True)
        try:
            cmds.ConvertSelectionToEdgePerimeter()
            cmds.ConvertSelectionToVertices()

            # List of pm vertex object
            vtxInOrder = self.getVerticesInOrder()

            pm.select(vtxInOrder, r=True)

            m_sel = OpenMaya.MSelectionList()
            OpenMaya.MGlobal.getActiveSelectionList(m_sel)
            dagPath = OpenMaya.MDagPath()
            components = OpenMaya.MObject()
            m_sel.getDagPath(0, dagPath, components)
            v_iter = OpenMaya.MItMeshVertex(dagPath, components)

            self.CENTER = self.getCenter(v_iter)

            fnMesh = OpenMaya.MFnMesh(dagPath)

            normalVector = self.getClosestNormal(self.CENTER, fnMesh)

            firstPoint = OpenMaya.MPoint(
                vtxInOrder[0].getPosition(space='world').x,
                vtxInOrder[0].getPosition(space='world').y,
                vtxInOrder[0].getPosition(space='world').z)

            firstVector = firstPoint - self.CENTER

            # Scale circle
            firstVector = firstVector * self.MULTIPLY

            numOfVertices = len(vtxInOrder)
            baseDegree = 360.0 / numOfVertices
            degree = 0.0 + self.ROTATION

            for i in vtxInOrder:
                nextVector = self.rotateVector(
                    degree, firstVector, normalVector)

                if self.INVERTED is True:
                    degree += -baseDegree
                else:
                    degree += baseDegree

                self.setPoint(i, nextVector, normalVector, fnMesh)
        except:
            pass
        finally:
            cmds.undoInfo(closeChunk=True)

    def setPoint(self, vertex, nextVector, normalVector, fnMesh):
        """ set point """

        pp = OpenMaya.MPoint(
            self.CENTER.x + nextVector.x,
            self.CENTER.y + nextVector.y,
            self.CENTER.z + nextVector.z)

        intersection = self.getIntersection(pp, normalVector, fnMesh)

        if intersection is None:
            cmds.warning("Failed")
            return

        vertex.setPosition(
            [intersection.x, intersection.y, intersection.z],
            space="world")

    def undoIt(self):
        pass

    def getVerticesInOrder(self):
        """ Get vertices in continous order """

        sel = pm.ls(sl=True, fl=True)
        left = list(sel)

        finished = [sel[0]]
        left.remove(sel[0])

        while True:
            currentVertex = finished[-1]
            distance = 999999.0
            closestVertex = None
            for v in left:
                a = v.getPosition(space='world')
                b = currentVertex.getPosition(space='world')
                length = (a - b).length()
                if length < distance:
                    distance = length
                    closestVertex = v
            finished.append(closestVertex)
            left.remove(closestVertex)
            if left == []:
                break

        return finished

    def getClosestNormal(self, mPoint, fnMesh):
        """ Get normal vector at the center"""

        normalVector = OpenMaya.MVector()

        ptr_int = self.UTIL.asIntPtr()

        fnMesh.getClosestNormal(
            mPoint,
            normalVector,
            OpenMaya.MSpace.kWorld,
            ptr_int)

        return normalVector

    def rotateVector(self, degree, vector, normalVector):
        """ Rotate vector """

        # nextVector = OpenMaya.MVector()
        rad = math.radians(degree)
        q1 = normalVector.x * math.sin(rad / 2)
        q2 = normalVector.y * math.sin(rad / 2)
        q3 = normalVector.z * math.sin(rad / 2)
        q4 = math.cos(rad / 2)

        return vector.rotateBy(q1, q2, q3, q4)

    def getCenter(self, vertex_iter):
        """ Get center point """

        mPointList = []

        while not vertex_iter.isDone():
            mPointList.append(vertex_iter.position(OpenMaya.MSpace.kWorld))
            vertex_iter.next()

        numOfVtx = len(mPointList)

        x = sum(mPointList[i].x for i in range(numOfVtx)) / numOfVtx
        y = sum(mPointList[i].y for i in range(numOfVtx)) / numOfVtx
        z = sum(mPointList[i].z for i in range(numOfVtx)) / numOfVtx

        return OpenMaya.MPoint(x, y, z)

    def getIntersection(self, point_in_3d, vector_in_3d, fnMesh):
        """ Return a point Position of intersection..
            Args:
                point_in_3d  (OpenMaya.MPoint)
                vector_in_3d (OpenMaya.MVector)
            Returns:
                OpenMaya.MFloatPoint : hitPoint
        """

        hitPoint = OpenMaya.MFloatPoint()
        hitFacePtr = self.UTIL.asIntPtr()
        idSorted = False
        testBothDirections = True
        faceIDs = None
        triIDs = None
        accelParam = None
        hitRayParam = None
        hitTriangle = None
        hitBary1 = None
        hitBary2 = None
        maxParamPtr = 99999

        result = fnMesh.closestIntersection(
            OpenMaya.MFloatPoint(
                point_in_3d.x,
                point_in_3d.y,
                point_in_3d.z),
            OpenMaya.MFloatVector(vector_in_3d),
            faceIDs,
            triIDs,
            idSorted,
            OpenMaya.MSpace.kWorld,
            maxParamPtr,
            testBothDirections,
            accelParam,
            hitPoint,
            hitRayParam,
            hitFacePtr,
            hitTriangle,
            hitBary1,
            hitBary2)

        if result is True:
            return hitPoint
        else:
            return None


# Creator
def cmdCreator():
    return OpenMayaMPx.asMPxPtr(CirculateVertices())


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
