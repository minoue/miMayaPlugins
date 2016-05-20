# DuplicateOnSurface


Duplicate object  over surface based on mouse clicks

![](https://dl.dropboxusercontent.com/u/408180/git/images/duplicateOnSurface.gif)

##  Flags
| Longname     | Shortname | Argument types | Default | Properties |
| :------- | :----: | :---: | :--: | :---: |
| rotation | r |  bool  | True  | C |

keep rotation if True. **Default is True**.

* `r=True`  
![](https://dl.dropboxusercontent.com/u/408180/git/images/dos_withRot.gif)

* `r=False`  
![](https://dl.dropboxusercontent.com/u/408180/git/images/dos_noRot.gif)

## Modifiers

**Scale** : Shift + drag to left/right

**Rotate** : Ctrl + drag

![](https://dl.dropboxusercontent.com/u/408180/git/images/dos_mod.gif)

**Snap to vertices** : Ctro + Shift

![](https://dl.dropboxusercontent.com/u/408180/git/images/dos_snap.gif)

## Python examples

```python
from maya import cmds
# Duplicate pCube1 over surface.
cmds.duplicateOnSurface("pCube1")

# Duplicate selected object over surface.
cmds.duplicateOnSurface(cmds.ls(sl=True, long=True)[0])

# Duplicate selected object over surface but keep original rotations.
cmds.duplicateOnSurface(cmds.ls(sl=True, long=True)[0], rotation=False)
```
