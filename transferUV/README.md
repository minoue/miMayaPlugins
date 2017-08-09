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

