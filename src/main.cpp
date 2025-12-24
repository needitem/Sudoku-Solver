#define UNICODE
#define _UNICODE
#include <windows.h>
#include "gui.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    return SudokuGUI::Run(hInstance, nCmdShow);
}
