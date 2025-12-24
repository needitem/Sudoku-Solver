#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <array>
#include <string>
#include <chrono>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

class SudokuSolver {
private:
    std::array<std::array<int, 9>, 9> grid;
    std::array<int, 9> rowMask, colMask, boxMask;

    int getBoxIndex(int row, int col) const { return (row / 3) * 3 + (col / 3); }
    int getCandidates(int row, int col) const {
        return ~(rowMask[row] | colMask[col] | boxMask[getBoxIndex(row, col)]) & 0x1FF;
    }
    int countBits(int mask) const {
        int count = 0;
        while (mask) { count++; mask &= (mask - 1); }
        return count;
    }

    void setCell(int row, int col, int num) {
        grid[row][col] = num;
        int bit = 1 << (num - 1);
        rowMask[row] |= bit; colMask[col] |= bit; boxMask[getBoxIndex(row, col)] |= bit;
    }

    void clearCell(int row, int col, int num) {
        grid[row][col] = 0;
        int bit = 1 << (num - 1);
        rowMask[row] &= ~bit; colMask[col] &= ~bit; boxMask[getBoxIndex(row, col)] &= ~bit;
    }

    std::pair<int, int> findBestCell() const {
        int minCand = 10, bestRow = -1, bestCol = -1;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (grid[i][j] == 0) {
                    int count = countBits(getCandidates(i, j));
                    if (count < minCand) { minCand = count; bestRow = i; bestCol = j; }
                }
            }
        }
        return {bestRow, bestCol};
    }

    bool nakedSingles() {
        bool progress = false;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (grid[i][j] == 0) {
                    int cand = getCandidates(i, j);
                    if (countBits(cand) == 1) {
                        int num = 1;
                        while (!(cand & 1)) { cand >>= 1; num++; }
                        setCell(i, j, num);
                        progress = true;
                    }
                }
            }
        }
        return progress;
    }

    bool hiddenSingles() {
        bool progress = false;
        for (int row = 0; row < 9; row++) {
            for (int num = 1; num <= 9; num++) {
                int bit = 1 << (num - 1);
                if (rowMask[row] & bit) continue;
                int possCol = -1, cnt = 0;
                for (int col = 0; col < 9 && cnt <= 1; col++)
                    if (grid[row][col] == 0 && (getCandidates(row, col) & bit)) { possCol = col; cnt++; }
                if (cnt == 1) { setCell(row, possCol, num); progress = true; }
            }
        }
        for (int col = 0; col < 9; col++) {
            for (int num = 1; num <= 9; num++) {
                int bit = 1 << (num - 1);
                if (colMask[col] & bit) continue;
                int possRow = -1, cnt = 0;
                for (int row = 0; row < 9 && cnt <= 1; row++)
                    if (grid[row][col] == 0 && (getCandidates(row, col) & bit)) { possRow = row; cnt++; }
                if (cnt == 1) { setCell(possRow, col, num); progress = true; }
            }
        }
        for (int br = 0; br < 3; br++) {
            for (int bc = 0; bc < 3; bc++) {
                for (int num = 1; num <= 9; num++) {
                    int bit = 1 << (num - 1);
                    if (boxMask[br * 3 + bc] & bit) continue;
                    int pr = -1, pc = -1, cnt = 0;
                    for (int i = 0; i < 3 && cnt <= 1; i++)
                        for (int j = 0; j < 3 && cnt <= 1; j++) {
                            int r = br * 3 + i, c = bc * 3 + j;
                            if (grid[r][c] == 0 && (getCandidates(r, c) & bit)) { pr = r; pc = c; cnt++; }
                        }
                    if (cnt == 1) { setCell(pr, pc, num); progress = true; }
                }
            }
        }
        return progress;
    }

    bool backtrack() {
        std::pair<int,int> best = findBestCell();
        int row = best.first, col = best.second;
        if (row == -1) return true;
        int cand = getCandidates(row, col);
        if (cand == 0) return false;
        for (int num = 1; num <= 9; num++) {
            if (cand & (1 << (num - 1))) {
                setCell(row, col, num);
                if (backtrack()) return true;
                clearCell(row, col, num);
            }
        }
        return false;
    }

public:
    std::array<std::array<bool, 9>, 9> isOriginal;

    SudokuSolver() { reset(); }

    void reset() {
        for (auto& row : grid) row.fill(0);
        for (auto& row : isOriginal) row.fill(false);
        rowMask.fill(0); colMask.fill(0); boxMask.fill(0);
    }

    bool isValidMove(int row, int col, int num) const {
        int bit = 1 << (num - 1);
        return !((rowMask[row] | colMask[col] | boxMask[getBoxIndex(row, col)]) & bit);
    }

    void setValue(int row, int col, int num, bool original = false) {
        if (grid[row][col] != 0) {
            int old = grid[row][col];
            int bit = 1 << (old - 1);
            rowMask[row] &= ~bit; colMask[col] &= ~bit; boxMask[getBoxIndex(row, col)] &= ~bit;
            grid[row][col] = 0;
        }
        if (num >= 1 && num <= 9) {
            setCell(row, col, num);
            if (original) isOriginal[row][col] = true;
        } else {
            isOriginal[row][col] = false;
        }
    }

    int getValue(int row, int col) const { return grid[row][col]; }

    bool solve() {
        bool progress = true;
        while (progress) {
            progress = nakedSingles() || hiddenSingles();
        }
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (grid[i][j] == 0) return backtrack();
        return true;
    }
};

// GUI 관련 전역 변수
SudokuSolver solver;
HWND cells[9][9];
HWND hStatus, hSolveBtn, hClearBtn;
HFONT hFont, hFontSolved;
HBRUSH hBrushWhite, hBrushBlue;
const int CELL_SIZE = 50;
const int MARGIN = 20;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CellProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

void UpdateDisplay(HWND hWnd) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int val = solver.getValue(i, j);
            if (val != 0) {
                wchar_t buf[2] = { static_cast<wchar_t>(L'0' + val), 0 };
                SetWindowText(cells[i][j], buf);
            } else {
                SetWindowText(cells[i][j], L"");
            }
            InvalidateRect(cells[i][j], NULL, TRUE);
        }
    }
}

void SolvePuzzle(HWND hWnd) {
    // solver 초기화 후 현재 입력값 다시 로드
    solver.reset();
    
    // 현재 UI에서 입력값 읽기
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            wchar_t buf[4] = {0};
            GetWindowText(cells[i][j], buf, 4);
            if (buf[0] >= L'1' && buf[0] <= L'9') {
                solver.setValue(i, j, buf[0] - L'0', true);
            }
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    bool solved = solver.solve();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    if (solved) {
        // 결과를 UI에 표시
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                int val = solver.getValue(i, j);
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
        SetWindowText(hStatus, L"해결할 수 없는 퍼즐입니다.");
    }
}

void ClearPuzzle(HWND hWnd) {
    solver.reset();
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            SetWindowText(cells[i][j], L"");
            InvalidateRect(cells[i][j], NULL, TRUE);
        }
    }
    SetWindowText(hStatus, L"숫자를 입력하고 '해결' 버튼을 누르세요");
}

LRESULT CALLBACK CellProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    int row = (int)(dwRefData / 9);
    int col = (int)(dwRefData % 9);

    switch (msg) {
    case WM_CHAR:
        if ((wParam >= '1' && wParam <= '9') || wParam == VK_BACK || wParam == 127) {
            if (wParam >= '1' && wParam <= '9') {
                int num = wParam - '0';
                int oldVal = solver.getValue(row, col);
                if (oldVal != 0) solver.setValue(row, col, 0);
                if (!solver.isValidMove(row, col, num)) {
                    if (oldVal != 0) solver.setValue(row, col, oldVal, solver.isOriginal[row][col]);
                    MessageBox(GetParent(hWnd), L"이 위치에 해당 숫자를 놓을 수 없습니다.", L"오류", MB_OK | MB_ICONWARNING);
                    return 0;
                }
                solver.setValue(row, col, num, true);
                wchar_t buf[2] = { static_cast<wchar_t>(wParam), 0 };
                SetWindowText(hWnd, buf);
                InvalidateRect(hWnd, NULL, TRUE);
                return 0;
            } else {
                solver.setValue(row, col, 0);
                SetWindowText(hWnd, L"");
                InvalidateRect(hWnd, NULL, TRUE);
                return 0;
            }
        }
        return 0;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        if (solver.isOriginal[row][col]) {
            SetTextColor(hdc, RGB(0, 0, 0));
        } else if (solver.getValue(row, col) != 0) {
            SetTextColor(hdc, RGB(0, 100, 200));
        }
        SetBkColor(hdc, RGB(255, 255, 255));
        return (LRESULT)hBrushWhite;
    }

    case WM_NCDESTROY:
        RemoveWindowSubclass(hWnd, CellProc, uIdSubclass);
        break;
    }
    return DefSubclassProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        hFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"맑은 고딕");
        hBrushWhite = CreateSolidBrush(RGB(255, 255, 255));
        hBrushBlue = CreateSolidBrush(RGB(220, 235, 255));

        // 타이틀
        HWND hTitle = CreateWindow(L"STATIC", L"스도쿠 솔버", WS_CHILD | WS_VISIBLE | SS_CENTER,
            MARGIN, 10, 9 * CELL_SIZE + 20, 30, hWnd, NULL, NULL, NULL);
        HFONT hTitleFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"맑은 고딕");
        SendMessage(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

        // 셀 생성
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
                    if (solver.isOriginal[i][j]) {
                        SetTextColor(hdc, RGB(0, 0, 0));
                    } else if (solver.getValue(i, j) != 0) {
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
        // 버튼 클릭만 처리 (BN_CLICKED)
        if (HIWORD(wParam) == BN_CLICKED) {
            if (LOWORD(wParam) == 1) SolvePuzzle(hWnd);
            else if (LOWORD(wParam) == 2) ClearPuzzle(hWnd);
        }
        break;

    case WM_DESTROY:
        DeleteObject(hFont);
        DeleteObject(hBrushWhite);
        DeleteObject(hBrushBlue);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
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
