# UV Checker

## Commands

### findUvOverlaps
#### Flags
| Longname | Shortname | Argument types | Default | Properties |
|:---------|----------:|:--------------:|:-------:|:----------:|
|multiThread|mt|bool|False|C|
|verbose|v|bool|False|C|

### checkUV
#### Flags
| Longname | Shortname | Argument types | Default | Properties |
|:---------|----------:|:--------------:|:-------:|:----------:|
|check|c|integer||C|
|verbose|v|bool|False|C|




## Examples
### UV Overlaps
< image here >

```
cmds.findUvOverlaps("|pSphere1")
```

### Reversed UVs
< image here >

```
cmds.findUvOverlaps("|pSphere1")
```

### UDIM border intersections
< image here >

```
cmds.checkUV("|pSphere1", c=0)
```

### No UVed Faces
< image here >

```
cmds.checkUV("|pSphere1", c=1)
```

### Zero Area UVs
< image here >

```
cmds.checkUV("|pSphere1", c=2)
```
