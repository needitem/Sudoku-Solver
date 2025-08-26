class SudokuGrid:
    def __init__(self):
        self.grid = [[0 for _ in range(9)] for _ in range(9)]
        self.candidates = [[set(range(1, 10)) if self.grid[i][j] == 0 else set() 
                           for j in range(9)] for i in range(9)]
    
    def set_value(self, row, col, value):
        if value == 0:
            self.grid[row][col] = 0
            self.candidates[row][col] = set(range(1, 10))
            self.update_candidates()
        else:
            self.grid[row][col] = value
            self.candidates[row][col] = set()
            self.propagate_constraints(row, col, value)
    
    def get_value(self, row, col):
        return self.grid[row][col]
    
    def is_valid_move(self, row, col, num):
        for c in range(9):
            if c != col and self.grid[row][c] == num:
                return False
        
        for r in range(9):
            if r != row and self.grid[r][col] == num:
                return False
        
        box_row, box_col = 3 * (row // 3), 3 * (col // 3)
        for r in range(box_row, box_row + 3):
            for c in range(box_col, box_col + 3):
                if (r != row or c != col) and self.grid[r][c] == num:
                    return False
        
        return True
    
    def propagate_constraints(self, row, col, value):
        for c in range(9):
            self.candidates[row][c].discard(value)
        
        for r in range(9):
            self.candidates[r][col].discard(value)
        
        box_row, box_col = 3 * (row // 3), 3 * (col // 3)
        for r in range(box_row, box_row + 3):
            for c in range(box_col, box_col + 3):
                self.candidates[r][c].discard(value)
    
    def update_candidates(self):
        self.candidates = [[set(range(1, 10)) if self.grid[i][j] == 0 else set() 
                           for j in range(9)] for i in range(9)]
        
        for row in range(9):
            for col in range(9):
                if self.grid[row][col] != 0:
                    self.propagate_constraints(row, col, self.grid[row][col])
    
    def get_candidates(self, row, col):
        return self.candidates[row][col]
    
    def is_complete(self):
        for row in range(9):
            for col in range(9):
                if self.grid[row][col] == 0:
                    return False
        return True
    
    def is_valid(self):
        for row in range(9):
            for col in range(9):
                if self.grid[row][col] == 0 and len(self.candidates[row][col]) == 0:
                    return False
        return True


class SudokuSolver:
    def __init__(self, grid):
        self.grid = grid
    
    def solve(self):
        progress = True
        while progress and not self.grid.is_complete():
            progress = False
            
            if self.naked_singles():
                progress = True
            
            if self.hidden_singles():
                progress = True
        
        if not self.grid.is_complete() and self.grid.is_valid():
            return self.backtrack()
        
        return self.grid.is_complete()
    
    def naked_singles(self):
        progress = False
        for row in range(9):
            for col in range(9):
                if self.grid.get_value(row, col) == 0:
                    candidates = self.grid.get_candidates(row, col)
                    if len(candidates) == 1:
                        value = next(iter(candidates))
                        self.grid.set_value(row, col, value)
                        progress = True
        return progress
    
    def hidden_singles(self):
        progress = False
        
        for row in range(9):
            for num in range(1, 10):
                possible_cols = []
                for col in range(9):
                    if self.grid.get_value(row, col) == 0 and num in self.grid.get_candidates(row, col):
                        possible_cols.append(col)
                
                if len(possible_cols) == 1:
                    col = possible_cols[0]
                    self.grid.set_value(row, col, num)
                    progress = True
        
        for col in range(9):
            for num in range(1, 10):
                possible_rows = []
                for row in range(9):
                    if self.grid.get_value(row, col) == 0 and num in self.grid.get_candidates(row, col):
                        possible_rows.append(row)
                
                if len(possible_rows) == 1:
                    row = possible_rows[0]
                    self.grid.set_value(row, col, num)
                    progress = True
        
        for box_row in range(3):
            for box_col in range(3):
                for num in range(1, 10):
                    possible_positions = []
                    for r in range(box_row * 3, box_row * 3 + 3):
                        for c in range(box_col * 3, box_col * 3 + 3):
                            if self.grid.get_value(r, c) == 0 and num in self.grid.get_candidates(r, c):
                                possible_positions.append((r, c))
                    
                    if len(possible_positions) == 1:
                        row, col = possible_positions[0]
                        self.grid.set_value(row, col, num)
                        progress = True
        
        return progress
    
    def backtrack(self):
        empty = self.find_empty_cell()
        if not empty:
            return True
        
        row, col = empty
        candidates = list(self.grid.get_candidates(row, col))
        
        for num in candidates:
            if self.grid.is_valid_move(row, col, num):
                old_grid = [row[:] for row in self.grid.grid]
                old_candidates = [[cell.copy() for cell in row] for row in self.grid.candidates]
                
                self.grid.set_value(row, col, num)
                
                if self.solve():
                    return True
                
                self.grid.grid = old_grid
                self.grid.candidates = old_candidates
        
        return False
    
    def find_empty_cell(self):
        min_candidates = 10
        best_cell = None
        
        for row in range(9):
            for col in range(9):
                if self.grid.get_value(row, col) == 0:
                    candidates_count = len(self.grid.get_candidates(row, col))
                    if candidates_count < min_candidates:
                        min_candidates = candidates_count
                        best_cell = (row, col)
        
        return best_cell