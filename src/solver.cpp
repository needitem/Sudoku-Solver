#include "solver.h"
#include "strategies.h"

SudokuSolver::SudokuSolver() : useBacktrack(false) { reset(); }

void SudokuSolver::reset() { grid.reset(); }

bool SudokuSolver::solve() {
    grid.updateCandidates();
    
    bool progress = true;
    while (progress && !grid.isComplete()) {
        progress = false;
        
        // 기본 전략
        if (Strategies::nakedSingles(grid)) { progress = true; continue; }
        if (Strategies::hiddenSingles(grid)) { progress = true; continue; }
        
        // 중급 전략
        if (Strategies::nakedPairs(grid)) { progress = true; continue; }
        if (Strategies::nakedTriples(grid)) { progress = true; continue; }
        if (Strategies::hiddenPairs(grid)) { progress = true; continue; }
        if (Strategies::hiddenTriples(grid)) { progress = true; continue; }
        if (Strategies::pointingPairs(grid)) { progress = true; continue; }
        if (Strategies::boxLineReduction(grid)) { progress = true; continue; }
        
        // 고급 전략
        if (Strategies::xWing(grid)) { progress = true; continue; }
        if (Strategies::swordfish(grid)) { progress = true; continue; }
        if (Strategies::yWing(grid)) { progress = true; continue; }
    }
    
    // 백트래킹 옵션
    if (!grid.isComplete() && useBacktrack) {
        return Strategies::backtrack(grid);
    }
    
    return grid.isComplete();
}
