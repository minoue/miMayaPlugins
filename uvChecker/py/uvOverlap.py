"""
uv overlap experiment script
currently very slow as this script checks using brute force
"""


from maya.api import OpenMaya
from maya import cmds
import time
import itertools


def getTriangleArea(x1, y1, x2, y2, x3, y3):
    """ Return area of triangle

    Args:
        x1 (float): x value of p1
        y1 (float): y value of p1
        x2 (float): x value of p2
        y2 (float): y value of p2
        x3 (float): x value of p3
        y3 (float): y value of p3

    Return:
        S (float): area

    """
    U = (x1 * y2) + (x2 * y3) + (x3 * y1) - (y1 * x2) - (y2 * x3) - (y3 * x1)
    S = U / 2.0
    return S


class Event(object):

    def __init__(self, status, point, edge, idx):
        """

        Args:
            status (str): status, eg, "begin", "end", or "cross"
            point (UvPoint): uv point.
            edge (UvEdge): containing uv edge
            idx (int): index

        """

        self.status = status
        self.eventPoint = point
        self.containedEdge = edge
        self.eventIndex = idx

        self.u = point.u
        self.v = point.v

    def __eq__(self, other):
        return self.eventIndex == other.eventIndex

    def __ne__(self, other):
        return self.eventIndex != other.eventIndex

    def __gt__(self, other):
        if self.u == other.u and self.v == other.v:
            return False
        elif self.v == other.v:
            return self.u > other.u
        else:
            return self.v > other.v

    def __ge__(self, other):
        if self.u == other.u and self.v == other.v:
            return True
        elif self.v == other.v:
            return self.u > other.u
        else:
            return self.v > other.v

    def __lt__(self, other):
        if self.u == other.u and self.v == other.v:
            return False
        elif self.v == other.v:
            return self.u < other.u
        else:
            return self.v < other.v

    def __le__(self, other):
        if self.u == other.u and self.v == other.v:
            return True
        elif self.v == other.v:
            return self.u < other.u
        else:
            return self.v < other.v


class Vector(object):

    def __init__(self, u, v):
        self.u = u
        self.v = v

    def dot(self, other):
        """ Get dot product

        Args:
            other (Vector): other vector

        Return:
            dot (float): dot product

        """
        dot = (self.u * other.u) + (self.v * other.v)
        return dot


class UvPoint(object):

    def __init__(self, u=None, v=None, index=None):
        self.u = u
        self.v = v
        self.index = index
        self.vertexIndex = None

    def __eq__(self, other):
        return self.index == other

    def __ne__(self, other):
        return self.index != other

    def __gt__(self, other):
        if self.u == other.u and self.v == other.v:
            return False
        elif self.v == other.v:
            return self.u > other.u
        else:
            return self.v > other.v

    def __ge__(self, other):
        if self.u == other.u and self.v == other.v:
            return True
        elif self.v == other.v:
            return self.u > other.u
        else:
            return self.v > other.v

    def __lt__(self, other):
        if self.u == other.u and self.v == other.v:
            return False
        elif self.v == other.v:
            return self.u < other.u
        else:
            return self.v < other.v

    def __le__(self, other):
        if self.u == other.u and self.v == other.v:
            return True
        elif self.v == other.v:
            return self.u < other.u
        else:
            return self.v < other.v

    def __sub__(self, other):
        u = other.u - self.u
        v = other.v - self.v
        vec = Vector(u, v)
        return vec


class UvEdge(object):

    def __init__(self, begin, end, index=None):
        self.begin = begin
        self.end = end
        self.index = index

    def __eq__(self, other):
        return self.index == other.index

    def __ne__(self, other):
        return self.index != other.index

    def isCrossing(self, other):
        area1 = getTriangleArea(
            self.begin.u,
            self.begin.v,
            other.begin.u,
            other.begin.v,
            self.end.u,
            self.end.v)

        area2 = getTriangleArea(
            self.begin.u,
            self.begin.v,
            other.end.u,
            other.end.v,
            self.end.u,
            self.end.v)

        area3 = getTriangleArea(
            other.begin.u,
            other.begin.v,
            self.begin.u,
            self.begin.v,
            other.end.u,
            other.end.v)

        area4 = getTriangleArea(
            other.begin.u,
            other.begin.v,
            self.end.u,
            self.end.v,
            other.end.u,
            other.end.v)

        ccw1 = area1 * area2
        ccw2 = area3 * area4

        if area1 == 0.0 and area2 == 0.0:
            # If two edges are parallel
            v1 = self.begin - other.begin
            v2 = self.end - other.begin
            d1 = v1.dot(v2)
            v3 = self.begin - other.end
            v4 = self.end - other.end
            d2 = v3.dot(v4)
            if d1 >= 0.0 and d2 >= 0.0:
                return False
            else:
                return True

        if ccw1 < 0 and ccw2 < 0:
            return True
        else:
            return False


class UvShell(object):

    def __init__(self):
        self.uvPoints = []
        self.shellId = None
        self.edges = set()

        self.uMax = None
        self.vMax = None
        self.uMin = None
        self.vMin = None


def main():
    mSel = OpenMaya.MGlobal.getActiveSelectionList()
    mDagPath = mSel.getDagPath(0)
    mFnMesh = OpenMaya.MFnMesh(mDagPath)

    numShells, uvShellIds = mFnMesh.getUvShellsIds()
    numPolygons = mFnMesh.numPolygons

    # Container for UV shell objects
    shells = []

    # Create shell objects and store them to the list
    for i in range(numShells):
        shell = UvShell()
        shell.shellId = i
        shells.append(shell)

    # Set max values
    for uvId, shellId in enumerate(uvShellIds):
        uv = mFnMesh.getUV(uvId)
        u = uv[0]
        v = uv[1]
        if shells[shellId].uMax is None:
            shells[shellId].uMax = u
        if u > shells[shellId].uMax:
            shells[shellId].uMax = u
        else:
            pass

        if shells[shellId].uMin is None:
            shells[shellId].uMin = u
        if u < shells[shellId].uMin:
            shells[shellId].uMin = u
        else:
            pass

        if shells[shellId].vMax is None:
            shells[shellId].vMax = v
        if v > shells[shellId].vMax:
            shells[shellId].vMax = v
        else:
            pass

        if shells[shellId].vMin is None:
            shells[shellId].vMin = v
        if v < shells[shellId].vMin:
            shells[shellId].vMin = v
        else:
            pass

    # Create UV edges
    for faceId in range(numPolygons):
        numPolygonVertices = mFnMesh.polygonVertexCount(faceId)
        for i in range(numPolygonVertices):
            if i == numPolygonVertices - 1:
                curLocalIndex = i
                nextLocalIndex = 0
            else:
                curLocalIndex = i
                nextLocalIndex = i + 1

            # UV indecis by local order
            uvIndexA = mFnMesh.getPolygonUVid(faceId, curLocalIndex)
            uvIndexB = mFnMesh.getPolygonUVid(faceId, nextLocalIndex)
            currentShellIndex = uvShellIds[uvIndexA]

            if uvIndexA < uvIndexB:
                edgeIndex = (uvIndexA, uvIndexB)
            else:
                edgeIndex = (uvIndexB, uvIndexA)

            # Get UV values and create edge objects
            u_current, v_current = mFnMesh.getPolygonUV(faceId, curLocalIndex)
            u_next, v_next = mFnMesh.getPolygonUV(faceId, nextLocalIndex)
            p1 = UvPoint(u_current, v_current, uvIndexA)
            p2 = UvPoint(u_next, v_next, uvIndexB)
            if p1 > p2:
                beginPt = p1
                endPt = p2
            else:
                beginPt = p2
                endPt = p1

            uvEdge = UvEdge(beginPt, endPt, edgeIndex)

            if uvEdge not in shells[currentShellIndex].edges:
                shells[currentShellIndex].edges.add(uvEdge)

    edgeSets = []

    for i in itertools.combinations(shells, 2):
        if shellOverlapped(i[0], i[1]):
            edges = []
            edges.extend(i[0].edges)
            edges.extend(i[1].edges)
            edgeSets.append(edges)
            try:
                shells.remove(i[0])
                shells.remove(i[1])
            except:
                pass
        else:
            pass

    result = []
    for i in edgeSets:
        r = bruteForceCheck(i)
        result.extend(r)

    for s in shells:
        r = bruteForceCheck(s.edges)
        result.extend(r)

    names = [mDagPath.fullPathName() + ".map[%s]" % i for i in result]

    if not len(names) == 0:
        cmds.select(names, r=True)


def shellOverlapped(shellA, shellB):
    if shellA.uMax < shellB.uMin:
        return False

    if shellA.uMin > shellB.uMax:
        return False

    if shellA.vMax < shellB.vMin:
        return False

    if shellA.vMin > shellB.vMax:
        return False

    return True


def bruteForceCheck(edges):
    result = []
    for i in itertools.combinations(edges, 2):
        pass
        if i[0].isCrossing(i[1]) is True:
            idx1 = i[0].index
            idx2 = i[1].index
            result.append(idx1)
            result.append(idx2)
    a = []
    for i in result:
        for s in i:
            a.append(s)

    return list(set(a))


def test():
    # Crossing points
    p1 = UvPoint(0.15, 0.1, 0)
    p2 = UvPoint(0.85, 0.5, 1)
    p3 = UvPoint(0.09, 0.9, 0)
    p4 = UvPoint(0.4, 0.55, 1)

    p5 = UvPoint(0.15, 0.1, 0)
    p6 = UvPoint(0.85, 0.5, 1)
    p7 = UvPoint(0.4, 0.95, 0)
    p8 = UvPoint(0.65, 0.15, 1)

    # parallel points
    p9 = UvPoint(0.15, 0.0, 0)
    p10 = UvPoint(0.50, 0.0, 1)
    p11 = UvPoint(0.4999, 0.0, 0)
    p12 = UvPoint(0.75, 0.0, 1)

    e1 = UvEdge(p1, p2, 0)
    e2 = UvEdge(p3, p4, 0)

    e3 = UvEdge(p5, p6, 0)
    e4 = UvEdge(p7, p8, 0)

    e5 = UvEdge(p9, p10, 0)
    e6 = UvEdge(p11, p12, 0)

    te = p9 - p12
    te2 = p10 - p12
    # print te.dot(te2)

    # print e1.isCrossing(e2)
    # print e3.isCrossing(e4)
    # print e5.isCrossing(e6)


if __name__ == "__main__":
    t = time.time()
    main()
    # test()
    print time.time() - t
