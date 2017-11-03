# topologyChecker

## Check numbers
0. Triangles
1. Ngons
2. Non-manifold edges
3. lamina faces
4. bi-valent faces
5. zero area faces
6. mesh border edges
7. crease edges


```python
from maya import cmds
e = cmds.checkTopology("pSphere1", c=0)
print e
[u'|pSphere1.f[360]', u'|pSphere1.f[361]', u'|pSphere1.f[362]', u'|pSphere1.f[363]', u'|pSphere1.f[364]', u'|pSphere1.f[365]', u'|pSphere1.f[366]', u'|pSphere1.f[367]', u'|pSphere1.f[368]', u'|pSphere1.f[369]', u'|pSphere1.f[370]', u'|pSphere1.f[371]', u'|pSphere1.f[372]', u'|pSphere1.f[373]', u'|pSphere1.f[374]', u'|pSphere1.f[375]', u'|pSphere1.f[376]', u'|pSphere1.f[377]', u'|pSphere1.f[378]', u'|pSphere1.f[379]', u'|pSphere1.f[380]', u'|pSphere1.f[381]', u'|pSphere1.f[382]', u'|pSphere1.f[383]', u'|pSphere1.f[384]', u'|pSphere1.f[385]', u'|pSphere1.f[386]', u'|pSphere1.f[387]', u'|pSphere1.f[388]', u'|pSphere1.f[389]', u'|pSphere1.f[390]', u'|pSphere1.f[391]', u'|pSphere1.f[392]', u'|pSphere1.f[393]', u'|pSphere1.f[394]', u'|pSphere1.f[395]', u'|pSphere1.f[396]', u'|pSphere1.f[397]', u'|pSphere1.f[398]', u'|pSphere1.f[399]']
```

