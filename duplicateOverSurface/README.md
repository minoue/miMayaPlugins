# DuplicateOverSurface


Duplicate object over surface based on mouse clicks.

![](https://dl.dropboxusercontent.com/u/408180/git/images/duplicateOnSurface.gif)

## Install
Copy duplicateOverSurface.py to you plug-ins directory.

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

**Snap to vertices** : Ctrl + Shift

![](https://dl.dropboxusercontent.com/u/408180/git/images/dos_snap.gif)

## Caution

### UVs  
**Target object must have UVs. If object has no UVs or bad UVs, the command will fail to execute or snap object will have unintentional deformation.  
スナップ対象のオブジェクトにUVが無いか、または不正なUVの場合、コマンドの実行に失敗もしくは複製オブジェクトが意図しない不必要な変形をします。**

### Surface normal
* Target normal soften  
![](https://dl.dropboxusercontent.com/u/408180/git/images/normalSoft.gif)
* Target normal harden  
![](https://dl.dropboxusercontent.com/u/408180/git/images/normalHard.gif)

## Python examples

```python
from maya import cmds
# Duplicate pCube1 over surface.
cmds.duplicateOverSurface("pCube1")

# Duplicate selected object over surface.
cmds.duplicateOverSurface(cmds.ls(sl=True, long=True)[0])

# Duplicate selected object over surface but keep original rotations.
cmds.duplicateOverSurface(cmds.ls(sl=True, long=True)[0], rotation=False)
```
