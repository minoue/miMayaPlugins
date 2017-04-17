# DuplicateOverSurface


Duplicate object over surface based on mouse clicks.

![](https://github.com/minoue/miMayaUtils/blob/media/duplicateOverSurface/dos_demo.gif)

## Install
Copy duplicateOverSurface.py to you plug-ins directory.

##  Flags
| Longname     | Shortname | Argument types | Default | Properties |
| :------- | :----: | :---: | :--: | :---: |
| rotation | r |  bool  | True  | C |

keep rotation if True. **Default is True**.

* `r=True`  
![](https://github.com/minoue/miMayaUtils/blob/media/duplicateOverSurface/dos_withRot.gif)

* `r=False`  
![](https://github.com/minoue/miMayaUtils/blob/media/duplicateOverSurface/dos_noRot.gif)

## Modifiers

**Scale** : Shift + drag to left/right

**Rotate** : Ctrl + drag

![](https://github.com/minoue/miMayaUtils/blob/media/duplicateOverSurface/dos_mod.gif)

**Snap to vertices** : Ctrl + Shift

![](https://github.com/minoue/miMayaUtils/blob/media/duplicateOverSurface/dos_snap.gif)

## Caution

### UVs  
**Target object must have UVs. If object has no UVs or bad UVs, the command will fail to execute or snap object will have unintentional deformation.  
スナップ対象のオブジェクトにUVが無いか、または不正なUVの場合、コマンドの実行に失敗もしくは複製オブジェクトが意図しない不必要な変形をします。**

### Surface normal
* Target normal soften  
![](https://github.com/minoue/miMayaUtils/blob/media/duplicateOverSurface/dos_normalSoft.gif)
* Target normal harden  
![](https://github.com/minoue/miMayaUtils/blob/media/duplicateOverSurface/dos_normalHard.gif)

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
