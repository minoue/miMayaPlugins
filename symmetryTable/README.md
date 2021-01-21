
## build

win

```
cd symmetryTable
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release -DMAYA_VERSION=2020 -DMAYA_ROOT_DIR="C:\Program Files\Autodesk\Maya2020" ../
cmake --build . --config Release --target install
```