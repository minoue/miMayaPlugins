# DuplicateOnSurface


Duplicate object  over surface based on mouse clicks

![](https://dl.dropboxusercontent.com/u/408180/git/images/duplicateOnSurface.gif)

##  Flags
| Longname     | Shortname | Argument types   | Properties |
| :------- | :----: | :---: | :---: |
| noRotation | nr |  bool    | C |

keep origianl rotation if True. **Default is False**.

* `nr=True`  
![](https://dl.dropboxusercontent.com/u/408180/git/images/dos_noRot.gif)

* `nr=False`  
![](https://dl.dropboxusercontent.com/u/408180/git/images/dos_withRot.gif)

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
cmds.duplicateOnSurface(cmds.ls(sl=True, long=True)[0], noRotation=True)
```
