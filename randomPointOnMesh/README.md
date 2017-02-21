```python
pp = cmds.randomPointOnMesh(n=1000)
pList = [pp[i:i+3] for i in range(0, len(pp), 3)]
```
