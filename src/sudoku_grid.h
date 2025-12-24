#pragma once
#include <array>

class SudokuGrid {
private:
    std::array<std::array<int, 9>, 9> grid;
    std::array<std::array<int, 9>, 9> candidates;
    std::array<int, 9> rowMask, colMask, boxMask;

public:
    std::array<std::array<bool, 9>, 9> isOriginal;

    SudokuGrid();
    void reset();
    
    int getBoxIndex(int row, int col) const;
    int getValue(int row, int col) const;
    int getCandidates(int row, int col) const;
    void setCandidates(int row, int col, int cand);
    
    void setCell(int row, int col, int num);
    void clearCell(int row, int col, int num);
    void setValue(int row, int col, int num, bool original = false);
    
    bool isValidMove(int row, int col, int num) const;
    bool isComplete() const;
    bool isValid() const;
    
    void updateCandidates();
    static int countBits(int mask);
    
    // 마스크 접근자
    int getRowMask(int row) const { return rowMask[row]; }
    int getColMask(int col) const { return colMask[col]; }
    int getBoxMask(int idx) const { return boxMask[idx]; }
};
