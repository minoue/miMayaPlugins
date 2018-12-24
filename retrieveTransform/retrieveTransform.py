from pymel import core as pm


def main():
    sel = pm.ls(os=True, fl=True)
    if len(sel) != 3:
        pm.warning("Select 3 vertices in order of origin, z, and x")
        return
    shape = sel[0].node()
    transformNode = pm.listRelatives(shape, parent=True)[0]
    piv = pm.xform(transformNode, q=True, ws=True, rp=True)
    p0 = sel[0].getPosition()
    p1 = sel[1].getPosition()
    p2 = sel[2].getPosition()
    Z = p1 - p0
    X = p2 - p0
    Y = Z ^ X
    Z.normalize()
    X.normalize()
    Y.normalize()
    P = pm.datatypes.Point(piv[0], piv[1], piv[2])

    M = pm.datatypes.Matrix(
        X.x, X.y, X.z, 0,
        Y.x, Y.y, Y.z, 0,
        Z.x, Z.y, Z.z, 0,
        P.x, P.y, P.z, 1)

    pm.xform(transformNode, matrix=M.inverse())
    pm.select(transformNode, r=True)
    pm.makeIdentity(apply=True, t=True, r=True, s=False, n=False)
    pm.xform(transformNode, ws=True, piv=(0, 0, 0))
    pm.xform(transformNode, matrix=M)


if __name__ == "__main__":
    main()
