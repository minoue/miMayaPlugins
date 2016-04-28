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
from maya import cmds
import math


DRAGGER = "duplicateOnClicked"
SOURCE = None
ANCHOR_POINT = None
MOD_POINT = None
MOD_FIRST = True
DUPLICATED = None
TARGET_FNMESH = None
SCALE_ORIG = None
SPACE = OpenMaya.MSpace.kWorld
UTIL = OpenMaya.MScriptUtil


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


def setupDragger():
    """ Setup dragger context command """

    try:
        cmds.deleteUI(DRAGGER)
    except:
        pass

    dragger = cmds.draggerContext(
        DRAGGER,
        pressCommand=pressEvent,
        dragCommand=dragEvent,
        releaseCommand=releaseEvent,
        space='screen',
        projection='viewPlane',
        undoMode='step',
        cursor='hand')

    return dragger


def run(source=None):
    """ Run command """

    if source is None:
        cmds.warning("Nothing is selected.")
        return

    global SOURCE
    SOURCE = source

    dragger = setupDragger()
    if dragger is None:
        return
    else:
        cmds.setToolTo(dragger)


def pressEvent():
    """ Event when click 3d view """

    button = cmds.draggerContext(DRAGGER, query=True, button=True)

    # Leave the tool by middle click
    if button == 2:
        cmds.setToolTo('selectSuperContext')
        return

    # Get clicked point in viewport screen space
    pressPosition = cmds.draggerContext(DRAGGER, query=True, ap=True)
    x = pressPosition[0]
    y = pressPosition[1]

    global ANCHOR_POINT
    ANCHOR_POINT = [x, y]

    # Convert
    point_in_3d, vector_in_3d = convertTo3D(x, y)

    # Get MFnMesh of snap target
    targetDagPath = getDagPathFromScreen(x, y)

    # If draggin outside of objects
    if targetDagPath is None:
        return

    # Create new object to snap
    global DUPLICATED
    DUPLICATED = getNewObject()
    print DUPLICATED

    # Get origianl scale information
    global SCALE_ORIG
    SCALE_ORIG = cmds.getAttr(DUPLICATED + ".scale")[0]

    global TARGET_FNMESH
    TARGET_FNMESH = OpenMaya.MFnMesh(targetDagPath)

    transformMatrix = getMatrix(
        point_in_3d,
        vector_in_3d,
        TARGET_FNMESH,
        SCALE_ORIG)

    # Reset transform of current object
    cmds.setAttr(DUPLICATED + ".translate", *[0, 0, 0])

    location = [-i for i in cmds.xform(DUPLICATED, q=True, ws=True, rp=True)]
    cmds.setAttr(DUPLICATED + ".translate", *location)
    cmds.makeIdentity(DUPLICATED, apply=True, t=True)

    # Apply transformMatrix to the new object
    cmds.xform(DUPLICATED, matrix=transformMatrix)


def getNewObject():
    print 'test'
    return cmds.duplicate(SOURCE)[0]


def dragEvent():
    """ Event while dragging a 3d view """

    if TARGET_FNMESH is None:
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
        global MOD_FIRST
        MOD_FIRST = True

    if modifier == "ctrl" or modifier == "shift":

        # If this is the first click of dragging
        if MOD_FIRST is True:
            global MOD_POINT
            MOD_POINT = [x, y]

            # global MOD_FIRST
            MOD_FIRST = False

        length, degree = getDragInfo(x, y)

        if modifier == "ctrl":
            length = 1.0
        elif modifier == "shift":
            degree = 0.0

        # Convert
        point_in_3d, vector_in_3d = convertTo3D(MOD_POINT[0], MOD_POINT[1])
    else:
        point_in_3d, vector_in_3d = convertTo3D(x, y)
        length = 1.0
        degree = 0.0

    # Get new transform matrix for new object
    transformMatrix = getMatrix(
        point_in_3d,
        vector_in_3d,
        TARGET_FNMESH,
        SCALE_ORIG,
        length,
        degree
        )

    if transformMatrix is None:
        return

    # Apply new transform
    cmds.xform(DUPLICATED, matrix=transformMatrix)
    cmds.setAttr(DUPLICATED + ".shear", *[0, 0, 0])

    cmds.refresh(currentView=True, force=True)


def getDragInfo(x, y):
    """ Get distance and angle in screen space. """

    start_x = MOD_POINT[0]
    start_y = MOD_POINT[1]
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


def releaseEvent():
    global MOD_FIRST
    MOD_FIRST = True


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


def getIntersection(point_in_3d, vector_in_3d, fnMesh):
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
        SPACE,
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
        None


def getNormal(pointPosition, targetFnMesh):
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
        SPACE,
        ptr_int)
    faceID = UTIL.getInt(ptr_int)
    normal.normalize()

    return normal, faceID


def getTangent(faceID, targetFnMesh):
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
        SPACE)
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


def getMatrix(mPoint,
              mVector,
              targetFnMesh,
              scale_orig,
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
    OP = getIntersection(mPoint, mVector, targetFnMesh)

    # If it doesn't intersect to any geometries, return None
    if OP is None:
        return None

    # Normal vector
    NV, faceID = getNormal(OP, targetFnMesh)

    # Tangent vector
    TV = getTangent(faceID, targetFnMesh)

    modifier = cmds.draggerContext(
        DRAGGER,
        query=True,
        modifier=True)

    # Ctrl-hold rotation
    if modifier == 'ctrl':
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
