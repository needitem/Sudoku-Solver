@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
cl /EHsc /O2 /utf-8 /I src src\main.cpp src\gui.cpp src\solver.cpp src\strategies.cpp src\sudoku_grid.cpp user32.lib gdi32.lib comctl32.lib /Fe:sudoku.exe
del *.obj 2>nul
