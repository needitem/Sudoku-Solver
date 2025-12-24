#include "strategies.h"
#include <vector>
#include <array>

namespace Strategies {

bool nakedSingles(SudokuGrid& grid) {
    bool progress = false;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (grid.getValue(i, j) == 0 && SudokuGrid::countBits(grid.getCandidates(i, j)) == 1) {
                int cand = grid.getCandidates(i, j);
                int num = 1;
                while (!(cand & 1)) { cand >>= 1; num++; }
                grid.setCell(i, j, num);
                progress = true;
            }
        }
    }
    return progress;
}

bool hiddenSingles(SudokuGrid& grid) {
    bool progress = false;
    // 행
    for (int row = 0; row < 9; row++) {
        for (int num = 1; num <= 9; num++) {
            int bit = 1 << (num - 1);
            if (grid.getRowMask(row) & bit) continue;
            int possCol = -1, cnt = 0;
            for (int col = 0; col < 9 && cnt <= 1; col++) {
                if (grid.getCandidates(row, col) & bit) { possCol = col; cnt++; }
            }
            if (cnt == 1) { grid.setCell(row, possCol, num); progress = true; }
        }
    }
    // 열
    for (int col = 0; col < 9; col++) {
        for (int num = 1; num <= 9; num++) {
            int bit = 1 << (num - 1);
            if (grid.getColMask(col) & bit) continue;
            int possRow = -1, cnt = 0;
            for (int row = 0; row < 9 && cnt <= 1; row++) {
                if (grid.getCandidates(row, col) & bit) { possRow = row; cnt++; }
            }
            if (cnt == 1) { grid.setCell(possRow, col, num); progress = true; }
        }
    }
    // 박스
    for (int br = 0; br < 3; br++) {
        for (int bc = 0; bc < 3; bc++) {
            for (int num = 1; num <= 9; num++) {
                int bit = 1 << (num - 1);
                if (grid.getBoxMask(br * 3 + bc) & bit) continue;
                int pr = -1, pc = -1, cnt = 0;
                for (int i = 0; i < 3 && cnt <= 1; i++) {
                    for (int j = 0; j < 3 && cnt <= 1; j++) {
                        if (grid.getCandidates(br * 3 + i, bc * 3 + j) & bit) { 
                            pr = br * 3 + i; pc = bc * 3 + j; cnt++; 
                        }
                    }
                }
                if (cnt == 1) { grid.setCell(pr, pc, num); progress = true; }
            }
        }
    }
    return progress;
}

bool nakedPairs(SudokuGrid& grid) {
    bool progress = false;
    // 행
    for (int row = 0; row < 9; row++) {
        for (int c1 = 0; c1 < 8; c1++) {
            if (grid.getValue(row, c1) != 0 || SudokuGrid::countBits(grid.getCandidates(row, c1)) != 2) continue;
            for (int c2 = c1 + 1; c2 < 9; c2++) {
                if (grid.getCandidates(row, c1) == grid.getCandidates(row, c2)) {
                    int pair = grid.getCandidates(row, c1);
                    for (int c = 0; c < 9; c++) {
                        if (c != c1 && c != c2 && grid.getValue(row, c) == 0) {
                            if (grid.getCandidates(row, c) & pair) {
                                grid.setCandidates(row, c, grid.getCandidates(row, c) & ~pair);
                                progress = true;
                            }
                        }
                    }
                }
            }
        }
    }
    // 열
    for (int col = 0; col < 9; col++) {
        for (int r1 = 0; r1 < 8; r1++) {
            if (grid.getValue(r1, col) != 0 || SudokuGrid::countBits(grid.getCandidates(r1, col)) != 2) continue;
            for (int r2 = r1 + 1; r2 < 9; r2++) {
                if (grid.getCandidates(r1, col) == grid.getCandidates(r2, col)) {
                    int pair = grid.getCandidates(r1, col);
                    for (int r = 0; r < 9; r++) {
                        if (r != r1 && r != r2 && grid.getValue(r, col) == 0) {
                            if (grid.getCandidates(r, col) & pair) {
                                grid.setCandidates(r, col, grid.getCandidates(r, col) & ~pair);
                                progress = true;
                            }
                        }
                    }
                }
            }
        }
    }
    // 박스
    for (int br = 0; br < 3; br++) {
        for (int bc = 0; bc < 3; bc++) {
            std::vector<std::pair<int, int>> cellsWithTwo;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int r = br * 3 + i, c = bc * 3 + j;
                    if (grid.getValue(r, c) == 0 && SudokuGrid::countBits(grid.getCandidates(r, c)) == 2) {
                        cellsWithTwo.push_back({r, c});
                    }
                }
            }
            for (size_t i = 0; i < cellsWithTwo.size(); i++) {
                for (size_t j = i + 1; j < cellsWithTwo.size(); j++) {
                    int r1 = cellsWithTwo[i].first, c1 = cellsWithTwo[i].second;
                    int r2 = cellsWithTwo[j].first, c2 = cellsWithTwo[j].second;
                    if (grid.getCandidates(r1, c1) == grid.getCandidates(r2, c2)) {
                        int pair = grid.getCandidates(r1, c1);
                        for (int ii = 0; ii < 3; ii++) {
                            for (int jj = 0; jj < 3; jj++) {
                                int r = br * 3 + ii, c = bc * 3 + jj;
                                if ((r != r1 || c != c1) && (r != r2 || c != c2) && grid.getValue(r, c) == 0) {
                                    if (grid.getCandidates(r, c) & pair) {
                                        grid.setCandidates(r, c, grid.getCandidates(r, c) & ~pair);
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

bool nakedTriples(SudokuGrid& grid) {
    bool progress = false;
    // 행
    for (int row = 0; row < 9; row++) {
        std::vector<int> cellsIdx;
        for (int c = 0; c < 9; c++) {
            int bits = SudokuGrid::countBits(grid.getCandidates(row, c));
            if (grid.getValue(row, c) == 0 && bits >= 2 && bits <= 3) {
                cellsIdx.push_back(c);
            }
        }
        for (size_t i = 0; i < cellsIdx.size(); i++) {
            for (size_t j = i + 1; j < cellsIdx.size(); j++) {
                for (size_t k = j + 1; k < cellsIdx.size(); k++) {
                    int c1 = cellsIdx[i], c2 = cellsIdx[j], c3 = cellsIdx[k];
                    int combined = grid.getCandidates(row, c1) | grid.getCandidates(row, c2) | grid.getCandidates(row, c3);
                    if (SudokuGrid::countBits(combined) == 3) {
                        for (int c = 0; c < 9; c++) {
                            if (c != c1 && c != c2 && c != c3 && grid.getValue(row, c) == 0) {
                                if (grid.getCandidates(row, c) & combined) {
                                    grid.setCandidates(row, c, grid.getCandidates(row, c) & ~combined);
                                    progress = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // 열
    for (int col = 0; col < 9; col++) {
        std::vector<int> cellsIdx;
        for (int r = 0; r < 9; r++) {
            int bits = SudokuGrid::countBits(grid.getCandidates(r, col));
            if (grid.getValue(r, col) == 0 && bits >= 2 && bits <= 3) {
                cellsIdx.push_back(r);
            }
        }
        for (size_t i = 0; i < cellsIdx.size(); i++) {
            for (size_t j = i + 1; j < cellsIdx.size(); j++) {
                for (size_t k = j + 1; k < cellsIdx.size(); k++) {
                    int r1 = cellsIdx[i], r2 = cellsIdx[j], r3 = cellsIdx[k];
                    int combined = grid.getCandidates(r1, col) | grid.getCandidates(r2, col) | grid.getCandidates(r3, col);
                    if (SudokuGrid::countBits(combined) == 3) {
                        for (int r = 0; r < 9; r++) {
                            if (r != r1 && r != r2 && r != r3 && grid.getValue(r, col) == 0) {
                                if (grid.getCandidates(r, col) & combined) {
                                    grid.setCandidates(r, col, grid.getCandidates(r, col) & ~combined);
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

bool hiddenPairs(SudokuGrid& grid) {
    bool progress = false;
    // 행
    for (int row = 0; row < 9; row++) {
        for (int n1 = 1; n1 <= 8; n1++) {
            int bit1 = 1 << (n1 - 1);
            if (grid.getRowMask(row) & bit1) continue;
            for (int n2 = n1 + 1; n2 <= 9; n2++) {
                int bit2 = 1 << (n2 - 1);
                if (grid.getRowMask(row) & bit2) continue;
                
                std::vector<int> positions;
                for (int c = 0; c < 9; c++) {
                    if (grid.getValue(row, c) == 0 && ((grid.getCandidates(row, c) & bit1) || (grid.getCandidates(row, c) & bit2))) {
                        if ((grid.getCandidates(row, c) & bit1) && (grid.getCandidates(row, c) & bit2)) {
                            positions.push_back(c);
                        } else {
                            positions.clear();
                            break;
                        }
                    }
                }
                if (positions.size() == 2) {
                    int pair = bit1 | bit2;
                    for (int c : positions) {
                        if (grid.getCandidates(row, c) != pair) {
                            grid.setCandidates(row, c, pair);
                            progress = true;
                        }
                    }
                }
            }
        }
    }
    // 열
    for (int col = 0; col < 9; col++) {
        for (int n1 = 1; n1 <= 8; n1++) {
            int bit1 = 1 << (n1 - 1);
            if (grid.getColMask(col) & bit1) continue;
            for (int n2 = n1 + 1; n2 <= 9; n2++) {
                int bit2 = 1 << (n2 - 1);
                if (grid.getColMask(col) & bit2) continue;
                
                std::vector<int> positions;
                for (int r = 0; r < 9; r++) {
                    if (grid.getValue(r, col) == 0 && ((grid.getCandidates(r, col) & bit1) || (grid.getCandidates(r, col) & bit2))) {
                        if ((grid.getCandidates(r, col) & bit1) && (grid.getCandidates(r, col) & bit2)) {
                            positions.push_back(r);
                        } else {
                            positions.clear();
                            break;
                        }
                    }
                }
                if (positions.size() == 2) {
                    int pair = bit1 | bit2;
                    for (int r : positions) {
                        if (grid.getCandidates(r, col) != pair) {
                            grid.setCandidates(r, col, pair);
                            progress = true;
                        }
                    }
                }
            }
        }
    }
    return progress;
}

bool hiddenTriples(SudokuGrid& grid) {
    bool progress = false;
    // 행
    for (int row = 0; row < 9; row++) {
        for (int n1 = 1; n1 <= 7; n1++) {
            int bit1 = 1 << (n1 - 1);
            if (grid.getRowMask(row) & bit1) continue;
            for (int n2 = n1 + 1; n2 <= 8; n2++) {
                int bit2 = 1 << (n2 - 1);
                if (grid.getRowMask(row) & bit2) continue;
                for (int n3 = n2 + 1; n3 <= 9; n3++) {
                    int bit3 = 1 << (n3 - 1);
                    if (grid.getRowMask(row) & bit3) continue;
                    
                    int triple = bit1 | bit2 | bit3;
                    std::vector<int> positions;
                    bool valid = true;
                    
                    for (int c = 0; c < 9 && valid; c++) {
                        if (grid.getValue(row, c) == 0 && (grid.getCandidates(row, c) & triple)) {
                            positions.push_back(c);
                            if (positions.size() > 3) valid = false;
                        }
                    }
                    
                    if (valid && positions.size() == 3) {
                        for (int c : positions) {
                            int newCand = grid.getCandidates(row, c) & triple;
                            if (grid.getCandidates(row, c) != newCand) {
                                grid.setCandidates(row, c, newCand);
                                progress = true;
                            }
                        }
                    }
                }
            }
        }
    }
    // 열
    for (int col = 0; col < 9; col++) {
        for (int n1 = 1; n1 <= 7; n1++) {
            int bit1 = 1 << (n1 - 1);
            if (grid.getColMask(col) & bit1) continue;
            for (int n2 = n1 + 1; n2 <= 8; n2++) {
                int bit2 = 1 << (n2 - 1);
                if (grid.getColMask(col) & bit2) continue;
                for (int n3 = n2 + 1; n3 <= 9; n3++) {
                    int bit3 = 1 << (n3 - 1);
                    if (grid.getColMask(col) & bit3) continue;
                    
                    int triple = bit1 | bit2 | bit3;
                    std::vector<int> positions;
                    bool valid = true;
                    
                    for (int r = 0; r < 9 && valid; r++) {
                        if (grid.getValue(r, col) == 0 && (grid.getCandidates(r, col) & triple)) {
                            positions.push_back(r);
                            if (positions.size() > 3) valid = false;
                        }
                    }
                    
                    if (valid && positions.size() == 3) {
                        for (int r : positions) {
                            int newCand = grid.getCandidates(r, col) & triple;
                            if (grid.getCandidates(r, col) != newCand) {
                                grid.setCandidates(r, col, newCand);
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

bool pointingPairs(SudokuGrid& grid) {
    bool progress = false;
    for (int br = 0; br < 3; br++) {
        for (int bc = 0; bc < 3; bc++) {
            for (int num = 1; num <= 9; num++) {
                int bit = 1 << (num - 1);
                if (grid.getBoxMask(br * 3 + bc) & bit) continue;
                
                int rows = 0, cols = 0;
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        if (grid.getCandidates(br * 3 + i, bc * 3 + j) & bit) {
                            rows |= (1 << i);
                            cols |= (1 << j);
                        }
                    }
                }
                if (SudokuGrid::countBits(rows) == 1) {
                    int ri = 0; while (!(rows & (1 << ri))) ri++;
                    int row = br * 3 + ri;
                    for (int c = 0; c < 9; c++) {
                        if (c / 3 != bc && (grid.getCandidates(row, c) & bit)) {
                            grid.setCandidates(row, c, grid.getCandidates(row, c) & ~bit);
                            progress = true;
                        }
                    }
                }
                if (SudokuGrid::countBits(cols) == 1) {
                    int ci = 0; while (!(cols & (1 << ci))) ci++;
                    int col = bc * 3 + ci;
                    for (int r = 0; r < 9; r++) {
                        if (r / 3 != br && (grid.getCandidates(r, col) & bit)) {
                            grid.setCandidates(r, col, grid.getCandidates(r, col) & ~bit);
                            progress = true;
                        }
                    }
                }
            }
        }
    }
    return progress;
}

bool boxLineReduction(SudokuGrid& grid) {
    bool progress = false;
    // 행
    for (int row = 0; row < 9; row++) {
        for (int num = 1; num <= 9; num++) {
            int bit = 1 << (num - 1);
            if (grid.getRowMask(row) & bit) continue;
            
            int boxFound = -1;
            bool multiBox = false;
            for (int c = 0; c < 9 && !multiBox; c++) {
                if (grid.getCandidates(row, c) & bit) {
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
                            if (grid.getCandidates(r, c) & bit) {
                                grid.setCandidates(r, c, grid.getCandidates(r, c) & ~bit);
                                progress = true;
                            }
                        }
                    }
                }
            }
        }
    }
    // 열
    for (int col = 0; col < 9; col++) {
        for (int num = 1; num <= 9; num++) {
            int bit = 1 << (num - 1);
            if (grid.getColMask(col) & bit) continue;
            
            int boxFound = -1;
            bool multiBox = false;
            for (int r = 0; r < 9 && !multiBox; r++) {
                if (grid.getCandidates(r, col) & bit) {
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
                            if (grid.getCandidates(r, c) & bit) {
                                grid.setCandidates(r, c, grid.getCandidates(r, c) & ~bit);
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

bool xWing(SudokuGrid& grid) {
    bool progress = false;
    for (int num = 1; num <= 9; num++) {
        int bit = 1 << (num - 1);
        
        // 행 기반
        for (int r1 = 0; r1 < 8; r1++) {
            std::vector<int> cols1;
            for (int c = 0; c < 9; c++) {
                if (grid.getCandidates(r1, c) & bit) cols1.push_back(c);
            }
            if (cols1.size() != 2) continue;
            
            for (int r2 = r1 + 1; r2 < 9; r2++) {
                std::vector<int> cols2;
                for (int c = 0; c < 9; c++) {
                    if (grid.getCandidates(r2, c) & bit) cols2.push_back(c);
                }
                if (cols2.size() == 2 && cols1[0] == cols2[0] && cols1[1] == cols2[1]) {
                    for (int r = 0; r < 9; r++) {
                        if (r != r1 && r != r2) {
                            if (grid.getCandidates(r, cols1[0]) & bit) {
                                grid.setCandidates(r, cols1[0], grid.getCandidates(r, cols1[0]) & ~bit);
                                progress = true;
                            }
                            if (grid.getCandidates(r, cols1[1]) & bit) {
                                grid.setCandidates(r, cols1[1], grid.getCandidates(r, cols1[1]) & ~bit);
                                progress = true;
                            }
                        }
                    }
                }
            }
        }
        
        // 열 기반
        for (int c1 = 0; c1 < 8; c1++) {
            std::vector<int> rows1;
            for (int r = 0; r < 9; r++) {
                if (grid.getCandidates(r, c1) & bit) rows1.push_back(r);
            }
            if (rows1.size() != 2) continue;
            
            for (int c2 = c1 + 1; c2 < 9; c2++) {
                std::vector<int> rows2;
                for (int r = 0; r < 9; r++) {
                    if (grid.getCandidates(r, c2) & bit) rows2.push_back(r);
                }
                if (rows2.size() == 2 && rows1[0] == rows2[0] && rows1[1] == rows2[1]) {
                    for (int c = 0; c < 9; c++) {
                        if (c != c1 && c != c2) {
                            if (grid.getCandidates(rows1[0], c) & bit) {
                                grid.setCandidates(rows1[0], c, grid.getCandidates(rows1[0], c) & ~bit);
                                progress = true;
                            }
                            if (grid.getCandidates(rows1[1], c) & bit) {
                                grid.setCandidates(rows1[1], c, grid.getCandidates(rows1[1], c) & ~bit);
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

bool swordfish(SudokuGrid& grid) {
    bool progress = false;
    for (int num = 1; num <= 9; num++) {
        int bit = 1 << (num - 1);
        
        // 행 기반
        std::vector<std::pair<int, std::vector<int>>> rowData;
        for (int r = 0; r < 9; r++) {
            std::vector<int> cols;
            for (int c = 0; c < 9; c++) {
                if (grid.getCandidates(r, c) & bit) cols.push_back(c);
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
                    
                    if (SudokuGrid::countBits(colMask) == 3) {
                        std::vector<int> sfCols;
                        for (int c = 0; c < 9; c++) {
                            if (colMask & (1 << c)) sfCols.push_back(c);
                        }
                        int r1 = rowData[i].first, r2 = rowData[j].first, r3 = rowData[k].first;
                        for (int r = 0; r < 9; r++) {
                            if (r != r1 && r != r2 && r != r3) {
                                for (int c : sfCols) {
                                    if (grid.getCandidates(r, c) & bit) {
                                        grid.setCandidates(r, c, grid.getCandidates(r, c) & ~bit);
                                        progress = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // 열 기반
        std::vector<std::pair<int, std::vector<int>>> colData;
        for (int c = 0; c < 9; c++) {
            std::vector<int> rows;
            for (int r = 0; r < 9; r++) {
                if (grid.getCandidates(r, c) & bit) rows.push_back(r);
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
                    
                    if (SudokuGrid::countBits(rowMaskSF) == 3) {
                        std::vector<int> sfRows;
                        for (int r = 0; r < 9; r++) {
                            if (rowMaskSF & (1 << r)) sfRows.push_back(r);
                        }
                        int c1 = colData[i].first, c2 = colData[j].first, c3 = colData[k].first;
                        for (int c = 0; c < 9; c++) {
                            if (c != c1 && c != c2 && c != c3) {
                                for (int r : sfRows) {
                                    if (grid.getCandidates(r, c) & bit) {
                                        grid.setCandidates(r, c, grid.getCandidates(r, c) & ~bit);
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

bool yWing(SudokuGrid& grid) {
    bool progress = false;
    
    struct WingCell { int row, col, cand; };
    
    for (int pr = 0; pr < 9; pr++) {
        for (int pc = 0; pc < 9; pc++) {
            if (grid.getValue(pr, pc) != 0 || SudokuGrid::countBits(grid.getCandidates(pr, pc)) != 2) continue;
            
            int pivot = grid.getCandidates(pr, pc);
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
                if (c != pc && grid.getValue(pr, c) == 0 && SudokuGrid::countBits(grid.getCandidates(pr, c)) == 2) {
                    wings.push_back({pr, c, grid.getCandidates(pr, c)});
                }
            }
            for (int r = 0; r < 9; r++) {
                if (r != pr && grid.getValue(r, pc) == 0 && SudokuGrid::countBits(grid.getCandidates(r, pc)) == 2) {
                    wings.push_back({r, pc, grid.getCandidates(r, pc)});
                }
            }
            int br = (pr / 3) * 3, bc = (pc / 3) * 3;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int r = br + i, c = bc + j;
                    if ((r != pr || c != pc) && r != pr && c != pc && 
                        grid.getValue(r, c) == 0 && SudokuGrid::countBits(grid.getCandidates(r, c)) == 2) {
                        wings.push_back({r, c, grid.getCandidates(r, c)});
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
                    
                    if (candC1 == candC2 && SudokuGrid::countBits(candC1) == 1) {
                        int bitC = candC1;
                        for (int r = 0; r < 9; r++) {
                            for (int c = 0; c < 9; c++) {
                                if (grid.getValue(r, c) != 0) continue;
                                bool sees1 = (r == r1 || c == c1 || (r / 3 == r1 / 3 && c / 3 == c1 / 3));
                                bool sees2 = (r == r2 || c == c2 || (r / 3 == r2 / 3 && c / 3 == c2 / 3));
                                if (sees1 && sees2 && (r != r1 || c != c1) && (r != r2 || c != c2)) {
                                    if (grid.getCandidates(r, c) & bitC) {
                                        grid.setCandidates(r, c, grid.getCandidates(r, c) & ~bitC);
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

bool backtrack(SudokuGrid& grid) {
    // 가장 후보가 적은 빈 셀 찾기
    int minCand = 10, bestRow = -1, bestCol = -1;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (grid.getValue(i, j) == 0) {
                int count = SudokuGrid::countBits(grid.getCandidates(i, j));
                if (count < minCand) {
                    minCand = count;
                    bestRow = i;
                    bestCol = j;
                }
            }
        }
    }
    
    if (bestRow == -1) return true;
    
    int cand = grid.getCandidates(bestRow, bestCol);
    if (cand == 0) return false;
    
    // 현재 상태 저장
    std::array<std::array<int, 9>, 9> savedGrid;
    std::array<std::array<int, 9>, 9> savedCandidates;
    
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            savedGrid[i][j] = grid.getValue(i, j);
            savedCandidates[i][j] = grid.getCandidates(i, j);
        }
    }
    
    for (int num = 1; num <= 9; num++) {
        if (cand & (1 << (num - 1))) {
            grid.setCell(bestRow, bestCol, num);
            if (backtrack(grid)) return true;
            
            // 상태 복원
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    if (savedGrid[i][j] == 0 && grid.getValue(i, j) != 0) {
                        grid.clearCell(i, j, grid.getValue(i, j));
                    }
                    grid.setCandidates(i, j, savedCandidates[i][j]);
                }
            }
        }
    }
    return false;
}

} // namespace Strategies