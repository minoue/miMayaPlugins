# transferUV
Yet another UV transfer command

## Why
Transfer attribute tool in maya is somehow very slow when transfering UVs between high res mesh. On the other hand, built-in command polyTransfer is very fast but it is unable to transfer only specific uv sets. 

## How to use 
1. Select source mesh, then target mesh
2. Run the following command

```python
from maya import cmds
cmds.transferUV(suv="map1", tuv="map1")
```

## Restriction
Two meshes have to have same point order/same number of vertices.

## Build
* MacOS/Linux (Maya2018)
```
mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DMAYA_VERSION=2018 ../
cmake --build . --config Release --target install
```
* Windows (Maya2018)
```
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release -DMAYA_VERSION=2018 ../
cmake --build . --config Release --target install
```

##  Flags

| Longname | Shortname | Argument types | default  | Properties |
| :------- |   :----:  |     :---:      |  :---:   |    :---:   |
| sourceUvSet |     suv     |     string     |          |      C     |
| targetUvSet | tuv | string |   | C |
| sourceMesh | sm | string | | C |
| targetMesh | tm | string | | C |