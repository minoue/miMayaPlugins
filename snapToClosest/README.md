# Snap to closest vertices/surface.

## Build
```
mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DMAYA_VERSION=2015 ../
cmake --build . --config Release --target install

```

##  Flags

| Longname | Shortname | Argument types | default  | Properties |
| :------- |   :----:  |     :---:      |  :---:   |    :---:   |
| mode     |     m     |     string     |          |      C     |
| searchDistance | d |  float | 10  | C |


## Python examples

Snap selected vertices to specified closest vertex or surface.

```cmds.snapToClosest("Plane1", mode="normal")```
![](https://raw.githubusercontent.com/wiki/minoue/miMayaUtils/images/normal.gif)


```cmds.snapToClosest("Plane1", mode="surface")```
![](https://raw.githubusercontent.com/wiki/minoue/miMayaUtils/images/surface.gif)


```cmds.snapToClosest("Plane1", mode="vertex")```
![](https://raw.githubusercontent.com/wiki/minoue/miMayaUtils/images/vertex.gif)


