#include "gui.h"
#include <chrono>

SudokuSolver SudokuGUI::solver;
HWND SudokuGUI::cells[9][9];
HWND SudokuGUI::hStatus;
HWND SudokuGUI::hSolveBtn;
HWND SudokuGUI::hClearBtn;
HFONT SudokuGUI::hFont;
HBRUSH SudokuGUI::hBrushWhite;

void SudokuGUI::SolvePuzzle(HWND hWnd) {
    solver.reset();
    
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            wchar_t buf[4] = {0};
            GetWindowText(cells[i][j], buf, 4);
            if (buf[0] >= L'1' && buf[0] <= L'9') {
                solver.getGrid().setValue(i, j, buf[0] - L'0', true);
            }
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    bool solved = solver.solve();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    if (solved) {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                int val = solver.getGrid().getValue(i, j);
                if (val != 0) {
                    wchar_t buf[2] = { static_cast<wchar_t>(L'0' + val), 0 };
                    SetWindowText(cells[i][j], buf);
                }
                InvalidateRect(cells[i][j], NULL, TRUE);
            }
        }
        wchar_t status[100];
        swprintf(status, 100, L"해결 완료! (%.3f ms)", duration.count() / 1000.0);
        SetWindowText(hStatus, status);
    } else {
        SetWindowText(hStatus, L"논리적 전략만으로는 해결할 수 없습니다.");
    }
}

void SudokuGUI::ClearPuzzle(HWND hWnd) {
    solver.reset();
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            SetWindowText(cells[i][j], L"");
            InvalidateRect(cells[i][j], NULL, TRUE);
        }
    }
    SetWindowText(hStatus, L"숫자를 입력하고 해결 버튼을 누르세요");
}

LRESULT CALLBACK SudokuGUI::CellProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    int row = (int)(dwRefData / 9);
    int col = (int)(dwRefData % 9);

    switch (msg) {
    case WM_CHAR:
        if ((wParam >= '1' && wParam <= '9') || wParam == VK_BACK || wParam == 127) {
            if (wParam >= '1' && wParam <= '9') {
                int num = wParam - '0';
                int oldVal = solver.getGrid().getValue(row, col);
                if (oldVal != 0) solver.getGrid().setValue(row, col, 0);
                if (!solver.getGrid().isValidMove(row, col, num)) {
                    if (oldVal != 0) solver.getGrid().setValue(row, col, oldVal, solver.getGrid().isOriginal[row][col]);
                    MessageBox(GetParent(hWnd), L"이 위치에 해당 숫자를 놓을 수 없습니다.", L"오류", MB_OK | MB_ICONWARNING);
                    return 0;
                }
                solver.getGrid().setValue(row, col, num, true);
                wchar_t buf[2] = { static_cast<wchar_t>(wParam), 0 };
                SetWindowText(hWnd, buf);
                InvalidateRect(hWnd, NULL, TRUE);
                return 0;
            } else {
                solver.getGrid().setValue(row, col, 0);
                SetWindowText(hWnd, L"");
                InvalidateRect(hWnd, NULL, TRUE);
                return 0;
            }
        }
        return 0;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hWnd, CellProc, uIdSubclass);
        break;
    }
    return DefSubclassProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK SudokuGUI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        hFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"맑은 고딕");
        hBrushWhite = CreateSolidBrush(RGB(255, 255, 255));

        HWND hTitle = CreateWindow(L"STATIC", L"스도쿠 솔버", WS_CHILD | WS_VISIBLE | SS_CENTER,
            MARGIN, 10, 9 * CELL_SIZE + 20, 30, hWnd, NULL, NULL, NULL);
        HFONT hTitleFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"맑은 고딕");
        SendMessage(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                int x = MARGIN + j * CELL_SIZE + (j / 3) * 4;
                int y = 50 + i * CELL_SIZE + (i / 3) * 4;
                
                cells[i][j] = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                    WS_CHILD | WS_VISIBLE | ES_CENTER | ES_NUMBER,
                    x, y, CELL_SIZE - 2, CELL_SIZE - 2, hWnd, (HMENU)(UINT_PTR)(i * 9 + j), NULL, NULL);
                
                SendMessage(cells[i][j], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(cells[i][j], EM_SETLIMITTEXT, 1, 0);
                SetWindowSubclass(cells[i][j], CellProc, i * 9 + j, i * 9 + j);
            }
        }

        int btnY = 60 + 9 * CELL_SIZE + 20;
        
        hSolveBtn = CreateWindow(L"BUTTON", L"해결", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            MARGIN + 80, btnY, 100, 35, hWnd, (HMENU)1, NULL, NULL);
        hClearBtn = CreateWindow(L"BUTTON", L"초기화", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            MARGIN + 200, btnY, 100, 35, hWnd, (HMENU)2, NULL, NULL);

        hStatus = CreateWindow(L"STATIC", L"숫자를 입력하고 해결 버튼을 누르세요",
            WS_CHILD | WS_VISIBLE | SS_CENTER, MARGIN, btnY + 50, 9 * CELL_SIZE + 20, 25, hWnd, NULL, NULL, NULL);

        HFONT hBtnFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"맑은 고딕");
        SendMessage(hSolveBtn, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
        SendMessage(hClearBtn, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
        SendMessage(hStatus, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
        break;
    }

    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        HWND hEdit = (HWND)lParam;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (cells[i][j] == hEdit) {
                    if (solver.getGrid().isOriginal[i][j]) {
                        SetTextColor(hdc, RGB(0, 0, 0));
                    } else if (solver.getGrid().getValue(i, j) != 0) {
                        SetTextColor(hdc, RGB(0, 100, 200));
                    } else {
                        SetTextColor(hdc, RGB(0, 0, 0));
                    }
                    SetBkColor(hdc, RGB(255, 255, 255));
                    return (LRESULT)hBrushWhite;
                }
            }
        }
        break;
    }

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) {
            if (LOWORD(wParam) == 1) SolvePuzzle(hWnd);
            else if (LOWORD(wParam) == 2) ClearPuzzle(hWnd);
        }
        break;

    case WM_DESTROY:
        DeleteObject(hFont);
        DeleteObject(hBrushWhite);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int SudokuGUI::Run(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance,
        LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1), NULL, L"SudokuClass", NULL };
    RegisterClassEx(&wc);

    int width = 9 * CELL_SIZE + MARGIN * 2 + 30;
    int height = 9 * CELL_SIZE + 180;

    HWND hWnd = CreateWindow(L"SudokuClass", L"스도쿠 솔버", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
