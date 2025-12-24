#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <array>
#include <string>
#include <chrono>
#include <vector>
#include <tuple>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

class SudokuSolver {
private:
    std::array<std::array<int, 9>, 9> grid;
    std::array<std::array<int, 9>, 9> candidates; // 각 셀의 후보 비트마스크
    std::array<int, 9> rowMask, colMask, boxMask;

    int getBoxIndex(int row, int col) const { return (row / 3) * 3 + (col / 3); }
    
    void updateCandidates() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (grid[i][j] == 0) {
                    candidates[i][j] = ~(rowMask[i] | colMask[j] | boxMask[getBoxIndex(i, j)]) & 0x1FF;
                } else {
                    candidates[i][j] = 0;
                }
            }
        }
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
        candidates[row][col] = 0;
        // 관련 셀들의 후보에서 제거
        for (int i = 0; i < 9; i++) {
            candidates[row][i] &= ~bit;
            candidates[i][col] &= ~bit;
        }
        int br = (row / 3) * 3, bc = (col / 3) * 3;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                candidates[br + i][bc + j] &= ~bit;
    }

    void clearCell(int row, int col, int num) {
        grid[row][col] = 0;
        int bit = 1 << (num - 1);
        rowMask[row] &= ~bit; colMask[col] &= ~bit; boxMask[getBoxIndex(row, col)] &= ~bit;
    }

    // 후보에서 특정 숫자 제거 (셀 값 설정 없이)
    bool eliminateCandidate(int row, int col, int bit) {
        if (candidates[row][col] & bit) {
            candidates[row][col] &= ~bit;
            return true;
        }
        return false;
    }

    // === 기본 전략 ===
    bool nakedSingles() {
        bool progress = false;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (grid[i][j] == 0 && countBits(candidates[i][j]) == 1) {
                    int cand = candidates[i][j];
                    int num = 1;
                    while (!(cand & 1)) { cand >>= 1; num++; }
                    setCell(i, j, num);
                    progress = true;
                }
            }
        }
        return progress;
    }

    bool hiddenSingles() {
        bool progress = false;
        // 행
        for (int row = 0; row < 9; row++) {
            for (int num = 1; num <= 9; num++) {
                int bit = 1 << (num - 1);
                if (rowMask[row] & bit) continue;
                int possCol = -1, cnt = 0;
                for (int col = 0; col < 9 && cnt <= 1; col++) {
                    if (candidates[row][col] & bit) { possCol = col; cnt++; }
                }
                if (cnt == 1) { setCell(row, possCol, num); progress = true; }
            }
        }
        // 열
        for (int col = 0; col < 9; col++) {
            for (int num = 1; num <= 9; num++) {
                int bit = 1 << (num - 1);
                if (colMask[col] & bit) continue;
                int possRow = -1, cnt = 0;
                for (int row = 0; row < 9 && cnt <= 1; row++) {
                    if (candidates[row][col] & bit) { possRow = row; cnt++; }
                }
                if (cnt == 1) { setCell(possRow, col, num); progress = true; }
            }
        }
        // 박스
        for (int br = 0; br < 3; br++) {
            for (int bc = 0; bc < 3; bc++) {
                for (int num = 1; num <= 9; num++) {
                    int bit = 1 << (num - 1);
                    if (boxMask[br * 3 + bc] & bit) continue;
                    int pr = -1, pc = -1, cnt = 0;
                    for (int i = 0; i < 3 && cnt <= 1; i++) {
                        for (int j = 0; j < 3 && cnt <= 1; j++) {
                            if (candidates[br * 3 + i][bc * 3 + j] & bit) { 
                                pr = br * 3 + i; pc = bc * 3 + j; cnt++; 
                            }
                        }
                    }
                    if (cnt == 1) { setCell(pr, pc, num); progress = true; }
                }
            }
        }
        return progress;
    }

    // === Naked Pairs ===
    bool nakedPairs() {
        bool progress = false;
        // 행에서 Naked Pairs
        for (int row = 0; row < 9; row++) {
            for (int c1 = 0; c1 < 8; c1++) {
                if (grid[row][c1] != 0 || countBits(candidates[row][c1]) != 2) continue;
                for (int c2 = c1 + 1; c2 < 9; c2++) {
                    if (candidates[row][c1] == candidates[row][c2]) {
                        int pair = candidates[row][c1];
                        for (int c = 0; c < 9; c++) {
                            if (c != c1 && c != c2 && grid[row][c] == 0) {
                                if (candidates[row][c] & pair) {
                                    candidates[row][c] &= ~pair;
                                    progress = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        // 열에서 Naked Pairs
        for (int col = 0; col < 9; col++) {
            for (int r1 = 0; r1 < 8; r1++) {
                if (grid[r1][col] != 0 || countBits(candidates[r1][col]) != 2) continue;
                for (int r2 = r1 + 1; r2 < 9; r2++) {
                    if (candidates[r1][col] == candidates[r2][col]) {
                        int pair = candidates[r1][col];
                        for (int r = 0; r < 9; r++) {
                            if (r != r1 && r != r2 && grid[r][col] == 0) {
                                if (candidates[r][col] & pair) {
                                    candidates[r][col] &= ~pair;
                                    progress = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        // 박스에서 Naked Pairs
        for (int br = 0; br < 3; br++) {
            for (int bc = 0; bc < 3; bc++) {
                std::vector<std::pair<int, int>> cellsWithTwo;
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        int r = br * 3 + i, c = bc * 3 + j;
                        if (grid[r][c] == 0 && countBits(candidates[r][c]) == 2) {
                            cellsWithTwo.push_back({r, c});
                        }
                    }
                }
                for (size_t i = 0; i < cellsWithTwo.size(); i++) {
                    for (size_t j = i + 1; j < cellsWithTwo.size(); j++) {
                        int r1 = cellsWithTwo[i].first, c1 = cellsWithTwo[i].second;
                        int r2 = cellsWithTwo[j].first, c2 = cellsWithTwo[j].second;
                        if (candidates[r1][c1] == candidates[r2][c2]) {
                            int pair = candidates[r1][c1];
                            for (int ii = 0; ii < 3; ii++) {
                                for (int jj = 0; jj < 3; jj++) {
                                    int r = br * 3 + ii, c = bc * 3 + jj;
                                    if ((r != r1 || c != c1) && (r != r2 || c != c2) && grid[r][c] == 0) {
                                        if (candidates[r][c] & pair) {
                                            candidates[r][c] &= ~pair;
                                            progress = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return progress;
    }

    // === Naked Triples ===
    bool nakedTriples() {
        bool progress = false;
        // 행에서 Naked Triples
        for (int row = 0; row < 9; row++) {
            std::vector<int> cellsIdx;
            for (int c = 0; c < 9; c++) {
                if (grid[row][c] == 0 && countBits(candidates[row][c]) >= 2 && countBits(candidates[row][c]) <= 3) {
                    cellsIdx.push_back(c);
                }
            }
            for (size_t i = 0; i < cellsIdx.size(); i++) {
                for (size_t j = i + 1; j < cellsIdx.size(); j++) {
                    for (size_t k = j + 1; k < cellsIdx.size(); k++) {
                        int c1 = cellsIdx[i], c2 = cellsIdx[j], c3 = cellsIdx[k];
                        int combined = candidates[row][c1] | candidates[row][c2] | candidates[row][c3];
                        if (countBits(combined) == 3) {
                            for (int c = 0; c < 9; c++) {
                                if (c != c1 && c != c2 && c != c3 && grid[row][c] == 0) {
                                    if (candidates[row][c] & combined) {
                                        candidates[row][c] &= ~combined;
                                        progress = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // 열에서 Naked Triples
        for (int col = 0; col < 9; col++) {
            std::vector<int> cellsIdx;
            for (int r = 0; r < 9; r++) {
                if (grid[r][col] == 0 && countBits(candidates[r][col]) >= 2 && countBits(candidates[r][col]) <= 3) {
                    cellsIdx.push_back(r);
                }
            }
            for (size_t i = 0; i < cellsIdx.size(); i++) {
                for (size_t j = i + 1; j < cellsIdx.size(); j++) {
                    for (size_t k = j + 1; k < cellsIdx.size(); k++) {
                        int r1 = cellsIdx[i], r2 = cellsIdx[j], r3 = cellsIdx[k];
                        int combined = candidates[r1][col] | candidates[r2][col] | candidates[r3][col];
                        if (countBits(combined) == 3) {
                            for (int r = 0; r < 9; r++) {
                                if (r != r1 && r != r2 && r != r3 && grid[r][col] == 0) {
                                    if (candidates[r][col] & combined) {
                                        candidates[r][col] &= ~combined;
                                        progress = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return progress;
    }

    // === Hidden Pairs ===
    bool hiddenPairs() {
        bool progress = false;
        // 행에서 Hidden Pairs
        for (int row = 0; row < 9; row++) {
            for (int n1 = 1; n1 <= 8; n1++) {
                int bit1 = 1 << (n1 - 1);
                if (rowMask[row] & bit1) continue;
                for (int n2 = n1 + 1; n2 <= 9; n2++) {
                    int bit2 = 1 << (n2 - 1);
                    if (rowMask[row] & bit2) continue;
                    
                    std::vector<int> positions;
                    for (int c = 0; c < 9; c++) {
                        if (grid[row][c] == 0 && ((candidates[row][c] & bit1) || (candidates[row][c] & bit2))) {
                            if ((candidates[row][c] & bit1) && (candidates[row][c] & bit2)) {
                                positions.push_back(c);
                            } else if ((candidates[row][c] & bit1) || (candidates[row][c] & bit2)) {
                                positions.clear();
                                break;
                            }
                        }
                    }
                    if (positions.size() == 2) {
                        int pair = bit1 | bit2;
                        for (int c : positions) {
                            if (candidates[row][c] != pair) {
                                candidates[row][c] = pair;
                                progress = true;
                            }
                        }
                    }
                }
            }
        }
        // 열에서 Hidden Pairs
        for (int col = 0; col < 9; col++) {
            for (int n1 = 1; n1 <= 8; n1++) {
                int bit1 = 1 << (n1 - 1);
                if (colMask[col] & bit1) continue;
                for (int n2 = n1 + 1; n2 <= 9; n2++) {
                    int bit2 = 1 << (n2 - 1);
                    if (colMask[col] & bit2) continue;
                    
                    std::vector<int> positions;
                    for (int r = 0; r < 9; r++) {
                        if (grid[r][col] == 0 && ((candidates[r][col] & bit1) || (candidates[r][col] & bit2))) {
                            if ((candidates[r][col] & bit1) && (candidates[r][col] & bit2)) {
                                positions.push_back(r);
                            } else if ((candidates[r][col] & bit1) || (candidates[r][col] & bit2)) {
                                positions.clear();
                                break;
                            }
                        }
                    }
                    if (positions.size() == 2) {
                        int pair = bit1 | bit2;
                        for (int r : positions) {
                            if (candidates[r][col] != pair) {
                                candidates[r][col] = pair;
                                progress = true;
                            }
                        }
                    }
                }
            }
        }
        return progress;
    }

    // === Hidden Triples ===
    bool hiddenTriples() {
        bool progress = false;
        // 행에서 Hidden Triples
        for (int row = 0; row < 9; row++) {
            for (int n1 = 1; n1 <= 7; n1++) {
                int bit1 = 1 << (n1 - 1);
                if (rowMask[row] & bit1) continue;
                for (int n2 = n1 + 1; n2 <= 8; n2++) {
                    int bit2 = 1 << (n2 - 1);
                    if (rowMask[row] & bit2) continue;
                    for (int n3 = n2 + 1; n3 <= 9; n3++) {
                        int bit3 = 1 << (n3 - 1);
                        if (rowMask[row] & bit3) continue;
                        
                        int triple = bit1 | bit2 | bit3;
                        std::vector<int> positions;
                        bool valid = true;
                        
                        for (int c = 0; c < 9 && valid; c++) {
                            if (grid[row][c] == 0 && (candidates[row][c] & triple)) {
                                positions.push_back(c);
                                if (positions.size() > 3) valid = false;
                            }
                        }
                        
                        if (valid && positions.size() == 3) {
                            for (int c : positions) {
                                int newCand = candidates[row][c] & triple;
                                if (candidates[row][c] != newCand) {
                                    candidates[row][c] = newCand;
                                    progress = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        // 열에서 Hidden Triples
        for (int col = 0; col < 9; col++) {
            for (int n1 = 1; n1 <= 7; n1++) {
                int bit1 = 1 << (n1 - 1);
                if (colMask[col] & bit1) continue;
                for (int n2 = n1 + 1; n2 <= 8; n2++) {
                    int bit2 = 1 << (n2 - 1);
                    if (colMask[col] & bit2) continue;
                    for (int n3 = n2 + 1; n3 <= 9; n3++) {
                        int bit3 = 1 << (n3 - 1);
                        if (colMask[col] & bit3) continue;
                        
                        int triple = bit1 | bit2 | bit3;
                        std::vector<int> positions;
                        bool valid = true;
                        
                        for (int r = 0; r < 9 && valid; r++) {
                            if (grid[r][col] == 0 && (candidates[r][col] & triple)) {
                                positions.push_back(r);
                                if (positions.size() > 3) valid = false;
                            }
                        }
                        
                        if (valid && positions.size() == 3) {
                            for (int r : positions) {
                                int newCand = candidates[r][col] & triple;
                                if (candidates[r][col] != newCand) {
                                    candidates[r][col] = newCand;
                                    progress = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        return progress;
    }

    // === Box-Line Reduction (역방향) ===
    bool boxLineReduction() {
        bool progress = false;
        // 행에서 숫자가 한 박스에만 있으면 박스 내 다른 셀에서 제거
        for (int row = 0; row < 9; row++) {
            for (int num = 1; num <= 9; num++) {
                int bit = 1 << (num - 1);
                if (rowMask[row] & bit) continue;
                
                int boxFound = -1;
                bool multiBox = false;
                for (int c = 0; c < 9 && !multiBox; c++) {
                    if (candidates[row][c] & bit) {
                        int box = c / 3;
                        if (boxFound == -1) boxFound = box;
                        else if (boxFound != box) multiBox = true;
                    }
                }
                
                if (!multiBox && boxFound != -1) {
                    int br = row / 3, bc = boxFound;
                    for (int i = 0; i < 3; i++) {
                        int r = br * 3 + i;
                        if (r != row) {
                            for (int j = 0; j < 3; j++) {
                                int c = bc * 3 + j;
                                if (candidates[r][c] & bit) {
                                    candidates[r][c] &= ~bit;
                                    progress = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        // 열에서 숫자가 한 박스에만 있으면 박스 내 다른 셀에서 제거
        for (int col = 0; col < 9; col++) {
            for (int num = 1; num <= 9; num++) {
                int bit = 1 << (num - 1);
                if (colMask[col] & bit) continue;
                
                int boxFound = -1;
                bool multiBox = false;
                for (int r = 0; r < 9 && !multiBox; r++) {
                    if (candidates[r][col] & bit) {
                        int box = r / 3;
                        if (boxFound == -1) boxFound = box;
                        else if (boxFound != box) multiBox = true;
                    }
                }
                
                if (!multiBox && boxFound != -1) {
                    int br = boxFound, bc = col / 3;
                    for (int j = 0; j < 3; j++) {
                        int c = bc * 3 + j;
                        if (c != col) {
                            for (int i = 0; i < 3; i++) {
                                int r = br * 3 + i;
                                if (candidates[r][c] & bit) {
                                    candidates[r][c] &= ~bit;
                                    progress = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        return progress;
    }

    // === Pointing Pairs (Box-Line Reduction) ===
    bool pointingPairs() {
        bool progress = false;
        for (int br = 0; br < 3; br++) {
            for (int bc = 0; bc < 3; bc++) {
                for (int num = 1; num <= 9; num++) {
                    int bit = 1 << (num - 1);
                    if (boxMask[br * 3 + bc] & bit) continue;
                    
                    int rows = 0, cols = 0;
                    for (int i = 0; i < 3; i++) {
                        for (int j = 0; j < 3; j++) {
                            if (candidates[br * 3 + i][bc * 3 + j] & bit) {
                                rows |= (1 << i);
                                cols |= (1 << j);
                            }
                        }
                    }
                    // 한 행에만 있으면 그 행의 다른 박스에서 제거
                    if (countBits(rows) == 1) {
                        int ri = 0; while (!(rows & (1 << ri))) ri++;
                        int row = br * 3 + ri;
                        for (int c = 0; c < 9; c++) {
                            if (c / 3 != bc && (candidates[row][c] & bit)) {
                                candidates[row][c] &= ~bit;
                                progress = true;
                            }
                        }
                    }
                    // 한 열에만 있으면 그 열의 다른 박스에서 제거
                    if (countBits(cols) == 1) {
                        int ci = 0; while (!(cols & (1 << ci))) ci++;
                        int col = bc * 3 + ci;
                        for (int r = 0; r < 9; r++) {
                            if (r / 3 != br && (candidates[r][col] & bit)) {
                                candidates[r][col] &= ~bit;
                                progress = true;
                            }
                        }
                    }
                }
            }
        }
        return progress;
    }

    // === X-Wing ===
    bool xWing() {
        bool progress = false;
        for (int num = 1; num <= 9; num++) {
            int bit = 1 << (num - 1);
            
            // 행 기반 X-Wing
            for (int r1 = 0; r1 < 8; r1++) {
                std::vector<int> cols1;
                for (int c = 0; c < 9; c++) {
                    if (candidates[r1][c] & bit) cols1.push_back(c);
                }
                if (cols1.size() != 2) continue;
                
                for (int r2 = r1 + 1; r2 < 9; r2++) {
                    std::vector<int> cols2;
                    for (int c = 0; c < 9; c++) {
                        if (candidates[r2][c] & bit) cols2.push_back(c);
                    }
                    if (cols2.size() == 2 && cols1[0] == cols2[0] && cols1[1] == cols2[1]) {
                        // X-Wing 발견! 해당 열의 다른 행에서 제거
                        for (int r = 0; r < 9; r++) {
                            if (r != r1 && r != r2) {
                                if (candidates[r][cols1[0]] & bit) {
                                    candidates[r][cols1[0]] &= ~bit;
                                    progress = true;
                                }
                                if (candidates[r][cols1[1]] & bit) {
                                    candidates[r][cols1[1]] &= ~bit;
                                    progress = true;
                                }
                            }
                        }
                    }
                }
            }
            
            // 열 기반 X-Wing
            for (int c1 = 0; c1 < 8; c1++) {
                std::vector<int> rows1;
                for (int r = 0; r < 9; r++) {
                    if (candidates[r][c1] & bit) rows1.push_back(r);
                }
                if (rows1.size() != 2) continue;
                
                for (int c2 = c1 + 1; c2 < 9; c2++) {
                    std::vector<int> rows2;
                    for (int r = 0; r < 9; r++) {
                        if (candidates[r][c2] & bit) rows2.push_back(r);
                    }
                    if (rows2.size() == 2 && rows1[0] == rows2[0] && rows1[1] == rows2[1]) {
                        for (int c = 0; c < 9; c++) {
                            if (c != c1 && c != c2) {
                                if (candidates[rows1[0]][c] & bit) {
                                    candidates[rows1[0]][c] &= ~bit;
                                    progress = true;
                                }
                                if (candidates[rows1[1]][c] & bit) {
                                    candidates[rows1[1]][c] &= ~bit;
                                    progress = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        return progress;
    }

    // === Swordfish ===
    bool swordfish() {
        bool progress = false;
        for (int num = 1; num <= 9; num++) {
            int bit = 1 << (num - 1);
            
            // 행 기반 Swordfish
            std::vector<std::pair<int, std::vector<int>>> rowData;
            for (int r = 0; r < 9; r++) {
                std::vector<int> cols;
                for (int c = 0; c < 9; c++) {
                    if (candidates[r][c] & bit) cols.push_back(c);
                }
                if (cols.size() >= 2 && cols.size() <= 3) {
                    rowData.push_back({r, cols});
                }
            }
            
            for (size_t i = 0; i < rowData.size(); i++) {
                for (size_t j = i + 1; j < rowData.size(); j++) {
                    for (size_t k = j + 1; k < rowData.size(); k++) {
                        int colMask = 0;
                        for (int c : rowData[i].second) colMask |= (1 << c);
                        for (int c : rowData[j].second) colMask |= (1 << c);
                        for (int c : rowData[k].second) colMask |= (1 << c);
                        
                        if (countBits(colMask) == 3) {
                            // Swordfish 발견
                            std::vector<int> sfCols;
                            for (int c = 0; c < 9; c++) {
                                if (colMask & (1 << c)) sfCols.push_back(c);
                            }
                            int r1 = rowData[i].first, r2 = rowData[j].first, r3 = rowData[k].first;
                            for (int r = 0; r < 9; r++) {
                                if (r != r1 && r != r2 && r != r3) {
                                    for (int c : sfCols) {
                                        if (candidates[r][c] & bit) {
                                            candidates[r][c] &= ~bit;
                                            progress = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // 열 기반 Swordfish
            std::vector<std::pair<int, std::vector<int>>> colData;
            for (int c = 0; c < 9; c++) {
                std::vector<int> rows;
                for (int r = 0; r < 9; r++) {
                    if (candidates[r][c] & bit) rows.push_back(r);
                }
                if (rows.size() >= 2 && rows.size() <= 3) {
                    colData.push_back({c, rows});
                }
            }
            
            for (size_t i = 0; i < colData.size(); i++) {
                for (size_t j = i + 1; j < colData.size(); j++) {
                    for (size_t k = j + 1; k < colData.size(); k++) {
                        int rowMaskSF = 0;
                        for (int r : colData[i].second) rowMaskSF |= (1 << r);
                        for (int r : colData[j].second) rowMaskSF |= (1 << r);
                        for (int r : colData[k].second) rowMaskSF |= (1 << r);
                        
                        if (countBits(rowMaskSF) == 3) {
                            std::vector<int> sfRows;
                            for (int r = 0; r < 9; r++) {
                                if (rowMaskSF & (1 << r)) sfRows.push_back(r);
                            }
                            int c1 = colData[i].first, c2 = colData[j].first, c3 = colData[k].first;
                            for (int c = 0; c < 9; c++) {
                                if (c != c1 && c != c2 && c != c3) {
                                    for (int r : sfRows) {
                                        if (candidates[r][c] & bit) {
                                            candidates[r][c] &= ~bit;
                                            progress = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return progress;
    }

    // === Y-Wing ===
    struct WingCell { int row, col, cand; };
    
    bool yWing() {
        bool progress = false;
        
        for (int pr = 0; pr < 9; pr++) {
            for (int pc = 0; pc < 9; pc++) {
                if (grid[pr][pc] != 0 || countBits(candidates[pr][pc]) != 2) continue;
                
                int pivot = candidates[pr][pc];
                int a = 0, b = 0;
                for (int n = 0; n < 9; n++) {
                    if (pivot & (1 << n)) {
                        if (a == 0) a = n + 1;
                        else b = n + 1;
                    }
                }
                int bitA = 1 << (a - 1), bitB = 1 << (b - 1);
                
                std::vector<WingCell> wings;
                
                for (int c = 0; c < 9; c++) {
                    if (c != pc && grid[pr][c] == 0 && countBits(candidates[pr][c]) == 2) {
                        wings.push_back({pr, c, candidates[pr][c]});
                    }
                }
                for (int r = 0; r < 9; r++) {
                    if (r != pr && grid[r][pc] == 0 && countBits(candidates[r][pc]) == 2) {
                        wings.push_back({r, pc, candidates[r][pc]});
                    }
                }
                int br = (pr / 3) * 3, bc = (pc / 3) * 3;
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        int r = br + i, c = bc + j;
                        if ((r != pr || c != pc) && r != pr && c != pc && 
                            grid[r][c] == 0 && countBits(candidates[r][c]) == 2) {
                            wings.push_back({r, c, candidates[r][c]});
                        }
                    }
                }
                
                for (size_t i = 0; i < wings.size(); i++) {
                    int r1 = wings[i].row, c1 = wings[i].col, cand1 = wings[i].cand;
                    if (!((cand1 & bitA) && !(cand1 & bitB))) continue;
                    int candC1 = cand1 & ~bitA;
                    
                    for (size_t j = 0; j < wings.size(); j++) {
                        if (i == j) continue;
                        int r2 = wings[j].row, c2 = wings[j].col, cand2 = wings[j].cand;
                        if (!((cand2 & bitB) && !(cand2 & bitA))) continue;
                        int candC2 = cand2 & ~bitB;
                        
                        if (candC1 == candC2 && countBits(candC1) == 1) {
                            int bitC = candC1;
                            for (int r = 0; r < 9; r++) {
                                for (int c = 0; c < 9; c++) {
                                    if (grid[r][c] != 0) continue;
                                    bool sees1 = (r == r1 || c == c1 || (r / 3 == r1 / 3 && c / 3 == c1 / 3));
                                    bool sees2 = (r == r2 || c == c2 || (r / 3 == r2 / 3 && c / 3 == c2 / 3));
                                    if (sees1 && sees2 && (r != r1 || c != c1) && (r != r2 || c != c2)) {
                                        if (candidates[r][c] & bitC) {
                                            candidates[r][c] &= ~bitC;
                                            progress = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return progress;
    }

public:
    std::array<std::array<bool, 9>, 9> isOriginal;

    SudokuSolver() { reset(); }

    void reset() {
        for (auto& row : grid) row.fill(0);
        for (auto& row : candidates) row.fill(0x1FF);
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
            clearCell(row, col, old);
            updateCandidates();
        }
        if (num >= 1 && num <= 9) {
            setCell(row, col, num);
            if (original) isOriginal[row][col] = true;
        } else {
            isOriginal[row][col] = false;
            updateCandidates();
        }
    }

    int getValue(int row, int col) const { return grid[row][col]; }

    bool isComplete() const {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (grid[i][j] == 0) return false;
        return true;
    }

    bool isValid() const {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (grid[i][j] == 0 && candidates[i][j] == 0) return false;
        return true;
    }

    bool solve() {
        updateCandidates();
        
        bool progress = true;
        while (progress && !isComplete()) {
            progress = false;
            
            // 기본 전략
            if (nakedSingles()) { progress = true; continue; }
            if (hiddenSingles()) { progress = true; continue; }
            
            // 중급 전략
            if (nakedPairs()) { progress = true; continue; }
            if (nakedTriples()) { progress = true; continue; }
            if (hiddenPairs()) { progress = true; continue; }
            if (hiddenTriples()) { progress = true; continue; }
            if (pointingPairs()) { progress = true; continue; }
            if (boxLineReduction()) { progress = true; continue; }
            
            // 고급 전략
            if (xWing()) { progress = true; continue; }
            if (swordfish()) { progress = true; continue; }
            if (yWing()) { progress = true; continue; }
        }
        
        return isComplete();
    }
};

// GUI 관련 전역 변수
SudokuSolver solver;
HWND cells[9][9];
HWND hStatus, hSolveBtn, hClearBtn;
HFONT hFont;
HBRUSH hBrushWhite;
const int CELL_SIZE = 50;
const int MARGIN = 20;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CellProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

void SolvePuzzle(HWND hWnd) {
    solver.reset();
    
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
        SetWindowText(hStatus, L"논리적 전략만으로는 해결할 수 없습니다.");
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
    SetWindowText(hStatus, L"숫자를 입력하고 해결 버튼을 누르세요");
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
