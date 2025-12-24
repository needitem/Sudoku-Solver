#pragma once
#include "sudoku_grid.h"

class SudokuSolver {
private:
    SudokuGrid grid;
    bool useBacktrack;

public:
    SudokuSolver();
    void reset();
    
    SudokuGrid& getGrid() { return grid; }
    const SudokuGrid& getGrid() const { return grid; }
    
    void setUseBacktrack(bool use) { useBacktrack = use; }
    bool getUseBacktrack() const { return useBacktrack; }
    
    bool solve();
};
