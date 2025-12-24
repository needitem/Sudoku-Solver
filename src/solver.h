#pragma once
#include "sudoku_grid.h"

class SudokuSolver {
private:
    SudokuGrid grid;

public:
    SudokuSolver();
    void reset();
    
    SudokuGrid& getGrid() { return grid; }
    const SudokuGrid& getGrid() const { return grid; }
    
    bool solve();
};
