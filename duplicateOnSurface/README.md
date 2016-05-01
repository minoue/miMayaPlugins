# DuplicateOnSurface


Duplicate object  over surface based on mouse clicks

![](https://dl.dropboxusercontent.com/u/408180/git/images/duplicateOnSurface.gif)

###  Flags
| longname     | shortname | Argument types   | Properties |
| :------- | :----: | :---: | :---: |
| noRotation | nr |  bool    | C |

keep origianl rotation if True. Default is False

False
![](https://dl.dropboxusercontent.com/u/408180/git/images/dos_noRot.gif)

 True
 ![](https://dl.dropboxusercontent.com/u/408180/git/images/dos_withRot.gif)

### Python examples

```python
from maya import cmds
# Duplicate pCube1 over surface.
cmds.duplicateOnSurface("pCube1")

# Duplicate selected object over surface.
cmds.duplicateOnSurface(cmds.ls(sl=True, long=True)[0])

# Duplicate selected object over surface but keep original rotations.
cmds.duplicateOnSurface(cmds.ls(sl=True, long=True)[0], noRotation=True)
```
