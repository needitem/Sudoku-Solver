import tkinter as tk
from tkinter import ttk, messagebox
from sudoku_logic import SudokuGrid, SudokuSolver


class SudokuGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("스도쿠 솔버")
        self.root.geometry("600x650")
        
        self.grid = SudokuGrid()
        self.solver = SudokuSolver(self.grid)
        
        self.create_widgets()
        self.create_grid()
        
    def create_widgets(self):
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        title_label = ttk.Label(main_frame, text="스도쿠 솔버", font=("Arial", 16, "bold"))
        title_label.grid(row=0, column=0, columnspan=3, pady=(0, 20))
        
        self.grid_frame = ttk.Frame(main_frame)
        self.grid_frame.grid(row=1, column=0, columnspan=3, pady=(0, 20))
        
        button_frame = ttk.Frame(main_frame)
        button_frame.grid(row=2, column=0, columnspan=3, pady=(10, 0))
        
        solve_button = ttk.Button(button_frame, text="해결", command=self.solve_sudoku)
        solve_button.grid(row=0, column=0, padx=(0, 10))
        
        clear_button = ttk.Button(button_frame, text="초기화", command=self.clear_grid)
        clear_button.grid(row=0, column=1, padx=(10, 0))
        
        self.status_label = ttk.Label(main_frame, text="초기 숫자를 입력하고 '해결' 버튼을 누르세요")
        self.status_label.grid(row=3, column=0, columnspan=3, pady=(20, 0))
        
    def create_grid(self):
        self.entries = []
        
        # 3x3 박스들을 위한 외부 프레임
        for box_row in range(3):
            for box_col in range(3):
                box_frame = tk.Frame(self.grid_frame, bg='black', bd=2, relief='solid')
                box_frame.grid(row=box_row, column=box_col, padx=2, pady=2)
                
                # 각 3x3 박스 내부의 셀들
                for i in range(3):
                    for j in range(3):
                        actual_row = box_row * 3 + i
                        actual_col = box_col * 3 + j
                        
                        # 첫 번째 행이면 entries 리스트에 새 행 추가
                        if actual_col == 0:
                            if actual_row >= len(self.entries):
                                self.entries.append([])
                        
                        # 셀 생성
                        entry = tk.Entry(box_frame, width=3, font=('Arial', 20, 'bold'), 
                                       justify='center', validate='key',
                                       validatecommand=(self.root.register(self.validate_input), '%P'),
                                       bg='white', fg='black', bd=1, relief='solid')
                        entry.grid(row=i, column=j, padx=1, pady=1, ipady=5)
                        
                        entry.bind('<FocusOut>', lambda e, r=actual_row, c=actual_col: self.update_grid_value(r, c))
                        entry.bind('<Return>', lambda e, r=actual_row, c=actual_col: self.update_grid_value(r, c))
                        
                        self.entries[actual_row].append(entry)
    
    def validate_input(self, value):
        if value == "":
            return True
        if value.isdigit() and 1 <= int(value) <= 9:
            return True
        return False
    
    def update_grid_value(self, row, col):
        entry = self.entries[row][col]
        value_str = entry.get()
        
        if value_str == "":
            value = 0
        else:
            value = int(value_str)
        
        if value != 0 and not self.grid.is_valid_move(row, col, value):
            messagebox.showerror("오류", f"({row+1}, {col+1}) 위치에 {value}은(는) 규칙에 맞지 않습니다.")
            entry.delete(0, tk.END)
            if self.grid.get_value(row, col) != 0:
                entry.insert(0, str(self.grid.get_value(row, col)))
            return
        
        self.grid.set_value(row, col, value)
        self.update_status()
    
    def solve_sudoku(self):
        self.status_label.config(text="해결 중...")
        self.root.update()
        
        original_grid = [row[:] for row in self.grid.grid]
        
        if self.solver.solve():
            self.update_display()
            self.status_label.config(text="해결 완료!")
            self.highlight_solution(original_grid)
        else:
            self.status_label.config(text="해결할 수 없는 스도쿠입니다.")
    
    def update_display(self):
        for i in range(9):
            for j in range(9):
                value = self.grid.get_value(i, j)
                self.entries[i][j].delete(0, tk.END)
                if value != 0:
                    self.entries[i][j].insert(0, str(value))
    
    def highlight_solution(self, original_grid):
        for i in range(9):
            for j in range(9):
                if original_grid[i][j] == 0 and self.grid.get_value(i, j) != 0:
                    self.entries[i][j].config(bg='lightblue', fg='blue')
                else:
                    self.entries[i][j].config(bg='white', fg='black')
    
    def clear_grid(self):
        for i in range(9):
            for j in range(9):
                self.entries[i][j].delete(0, tk.END)
                self.entries[i][j].config(bg='white', fg='black')
                self.grid.set_value(i, j, 0)
        
        self.status_label.config(text="그리드가 초기화되었습니다.")
    
    
    def update_status(self):
        empty_count = sum(1 for i in range(9) for j in range(9) if self.grid.get_value(i, j) == 0)
        if empty_count == 0:
            self.status_label.config(text="완성되었습니다!")
        else:
            self.status_label.config(text=f"빈 칸: {empty_count}개")