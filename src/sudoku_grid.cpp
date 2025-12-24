#include "sudoku_grid.h"

SudokuGrid::SudokuGrid() { reset(); }

void SudokuGrid::reset() {
    for (auto& row : grid) row.fill(0);
    for (auto& row : candidates) row.fill(0x1FF);
    for (auto& row : isOriginal) row.fill(false);
    rowMask.fill(0); colMask.fill(0); boxMask.fill(0);
}

int SudokuGrid::getBoxIndex(int row, int col) const { 
    return (row / 3) * 3 + (col / 3); 
}

int SudokuGrid::getValue(int row, int col) const { 
    return grid[row][col]; 
}

int SudokuGrid::getCandidates(int row, int col) const { 
    return candidates[row][col]; 
}

void SudokuGrid::setCandidates(int row, int col, int cand) { 
    candidates[row][col] = cand; 
}

void SudokuGrid::setCell(int row, int col, int num) {
    grid[row][col] = num;
    int bit = 1 << (num - 1);
    rowMask[row] |= bit; 
    colMask[col] |= bit; 
    boxMask[getBoxIndex(row, col)] |= bit;
    candidates[row][col] = 0;
    
    for (int i = 0; i < 9; i++) {
        candidates[row][i] &= ~bit;
        candidates[i][col] &= ~bit;
    }
    int br = (row / 3) * 3, bc = (col / 3) * 3;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            candidates[br + i][bc + j] &= ~bit;
}

void SudokuGrid::clearCell(int row, int col, int num) {
    grid[row][col] = 0;
    int bit = 1 << (num - 1);
    rowMask[row] &= ~bit; 
    colMask[col] &= ~bit; 
    boxMask[getBoxIndex(row, col)] &= ~bit;
}

void SudokuGrid::setValue(int row, int col, int num, bool original) {
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

bool SudokuGrid::isValidMove(int row, int col, int num) const {
    int bit = 1 << (num - 1);
    return !((rowMask[row] | colMask[col] | boxMask[getBoxIndex(row, col)]) & bit);
}

bool SudokuGrid::isComplete() const {
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (grid[i][j] == 0) return false;
    return true;
}

bool SudokuGrid::isValid() const {
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (grid[i][j] == 0 && candidates[i][j] == 0) return false;
    return true;
}

void SudokuGrid::updateCandidates() {
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

int SudokuGrid::countBits(int mask) {
    int count = 0;
    while (mask) { count++; mask &= (mask - 1); }
    return count;
}
