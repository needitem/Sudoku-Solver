# 스도쿠 솔버 (Sudoku Solver)

Python Tkinter를 사용한 GUI 기반 스도쿠 해결 프로그램입니다.

## 실행 방법

```bash
python sudoku_solver.py
```

## 알고리즘

### 1. Constraint Propagation (제약 전파)

각 셀마다 가능한 후보 숫자들(1-9)을 추적하고, 숫자가 배치될 때마다 관련된 행, 열, 3x3 박스의 다른 셀들에서 해당 숫자를 후보에서 제거합니다.

```python
def propagate_constraints(self, row, col, value):
    # 같은 행의 모든 셀에서 value 제거
    for c in range(9):
        self.candidates[row][c].discard(value)
    
    # 같은 열의 모든 셀에서 value 제거
    for r in range(9):
        self.candidates[r][col].discard(value)
    
    # 같은 3x3 박스의 모든 셀에서 value 제거
    box_row, box_col = 3 * (row // 3), 3 * (col // 3)
    for r in range(box_row, box_row + 3):
        for c in range(box_col, box_col + 3):
            self.candidates[r][c].discard(value)
```

### 2. Naked Singles

특정 셀에 하나의 숫자만 들어갈 수 있는 경우를 찾아 해결합니다.

```python
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
```

**예시:** (0,0) 셀의 후보가 {7}만 남은 경우, 해당 셀의 값을 7로 확정

### 3. Hidden Singles

특정 숫자가 행, 열, 또는 3x3 박스에서 한 곳에만 들어갈 수 있는 경우를 찾아 해결합니다.

```python
def hidden_singles(self):
    progress = False
    
    # 행에서 Hidden Singles 찾기
    for row in range(9):
        for num in range(1, 10):
            possible_cols = []
            for col in range(9):
                if (self.grid.get_value(row, col) == 0 and 
                    num in self.grid.get_candidates(row, col)):
                    possible_cols.append(col)
            
            if len(possible_cols) == 1:
                col = possible_cols[0]
                self.grid.set_value(row, col, num)
                progress = True
    
    # 열과 3x3 박스에서도 동일하게 수행...
    return progress
```

**예시:** 첫 번째 행에서 숫자 5가 마지막 칸에만 들어갈 수 있는 경우, 해당 칸을 5로 확정

### 4. Backtracking

위의 논리적 방법들로 해결되지 않는 경우 시행착오 방식을 사용합니다.

```python
def backtrack(self):
    empty = self.find_empty_cell()  # 후보가 가장 적은 셀 선택
    if not empty:
        return True
    
    row, col = empty
    candidates = list(self.grid.get_candidates(row, col))
    
    for num in candidates:
        if self.grid.is_valid_move(row, col, num):
            # 현재 상태 저장
            old_grid = [row[:] for row in self.grid.grid]
            old_candidates = [[cell.copy() for cell in row] 
                            for row in self.grid.candidates]
            
            # 숫자 배치 시도
            self.grid.set_value(row, col, num)
            
            if self.solve():
                return True
            
            # 실패시 이전 상태로 복원
            self.grid.grid = old_grid
            self.grid.candidates = old_candidates
    
    return False
```

### 해결 과정

1. **Constraint Propagation**: 초기 숫자들을 바탕으로 각 셀의 후보 숫자 계산
2. **Naked Singles**: 후보가 하나인 셀들을 찾아 확정
3. **Hidden Singles**: 행/열/박스에서 유일한 위치에만 올 수 있는 숫자들을 찾아 확정
4. **반복**: 2-3번을 더 이상 진전이 없을 때까지 반복
5. **Backtracking**: 논리적 방법으로 해결되지 않으면 시행착오 방식 적용

### 최적화 기법

- **MRV (Minimum Remaining Values)**: Backtracking 시 후보가 가장 적은 셀부터 선택
- **상태 복원**: 실패 시 이전 상태로 효율적으로 되돌리기
- **조기 종료**: 유효하지 않은 상태 감지 시 즉시 백트랙

## 파일 구조

- `sudoku_solver.py`: 메인 실행 파일
- `sudoku_logic.py`: 스도쿠 해결 알고리즘 구현
- `sudoku_gui.py`: Tkinter GUI 구현

## 사용법

1. 프로그램 실행 후 초기 숫자들을 입력
2. "해결" 버튼 클릭
3. 해결된 숫자들은 파란색으로 표시됨
4. "초기화" 버튼으로 그리드 리셋 가능