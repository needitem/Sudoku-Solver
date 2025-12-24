#pragma once

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include "solver.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

class SudokuGUI {
private:
    static SudokuSolver solver;
    static HWND cells[9][9];
    static HWND hStatus, hSolveBtn, hClearBtn;
    static HFONT hFont;
    static HBRUSH hBrushWhite;
    
    static const int CELL_SIZE = 50;
    static const int MARGIN = 20;

public:
    static void SolvePuzzle(HWND hWnd);
    static void ClearPuzzle(HWND hWnd);
    
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK CellProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    
    static int Run(HINSTANCE hInstance, int nCmdShow);
};
