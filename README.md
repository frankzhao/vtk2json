vtk2json
========

Converts VTK to JSON for the three.js JSONLoader

Currently only supports ASCII VTK files. Compile using g++.
Uses GNU++98 `[-std=gnu++98]` and `libstdc++`.

Compiling
---
1. `./compile.sh` or `g++ vtk2json.cpp -O3 -o vtk2json`
2. Run `vtk2json [vtk file] [output file]`
