#pragma once
#include "sudoku_grid.h"

namespace Strategies {
    // 기본 전략
    bool nakedSingles(SudokuGrid& grid);
    bool hiddenSingles(SudokuGrid& grid);
    
    // 중급 전략
    bool nakedPairs(SudokuGrid& grid);
    bool nakedTriples(SudokuGrid& grid);
    bool hiddenPairs(SudokuGrid& grid);
    bool hiddenTriples(SudokuGrid& grid);
    bool pointingPairs(SudokuGrid& grid);
    bool boxLineReduction(SudokuGrid& grid);
    
    // 고급 전략
    bool xWing(SudokuGrid& grid);
    bool swordfish(SudokuGrid& grid);
    bool yWing(SudokuGrid& grid);
}
