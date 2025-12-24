# 스도쿠 솔버 (Sudoku Solver)

C++ Win32 네이티브 GUI 스도쿠 해결 프로그램입니다. 백트래킹 없이 논리적 전략만으로 퍼즐을 해결합니다.

## 다운로드

[Releases](https://github.com/needitem/Sudoku-Solver/releases)에서 `sudoku.exe` 다운로드

## 해결 전략

### 기본 전략

| 전략 | 설명 |
|------|------|
| Naked Singles | 셀에 후보가 하나만 남으면 확정 |
| Hidden Singles | 행/열/박스에서 숫자가 한 곳에만 가능하면 확정 |

### 중급 전략

| 전략 | 설명 |
|------|------|
| Naked Pairs | 같은 2개 후보를 가진 셀 2개 → 다른 셀에서 제거 |
| Naked Triples | 3개 셀의 후보 합집합이 3개 → 다른 셀에서 제거 |
| Hidden Pairs | 2개 숫자가 2개 셀에만 존재 → 그 셀의 다른 후보 제거 |
| Hidden Triples | 3개 숫자가 3개 셀에만 존재 → 그 셀의 다른 후보 제거 |
| Pointing Pairs | 박스 내 숫자가 한 행/열에만 → 행/열의 박스 외부에서 제거 |
| Box-Line Reduction | 행/열의 숫자가 한 박스에만 → 박스 내 다른 셀에서 제거 |

### 고급 전략

| 전략 | 설명 |
|------|------|
| X-Wing | 2행에서 숫자가 같은 2열에만 → 해당 열의 다른 행에서 제거 |
| Swordfish | X-Wing의 3행/3열 확장 |
| Y-Wing | 피벗(AB) + 날개(AC, BC) → 두 날개가 보는 셀에서 C 제거 |

## 전략 적용 순서

```
기본: Naked Singles → Hidden Singles
  ↓
중급: Naked Pairs → Naked Triples → Hidden Pairs → Hidden Triples
      → Pointing Pairs → Box-Line Reduction
  ↓
고급: X-Wing → Swordfish → Y-Wing
  ↓
  (반복)
```

## 사용법

1. 숫자 입력 (1-9)
2. "해결" 클릭
3. 파란색 = 솔버가 채운 숫자

## 빌드

```cmd
cl /EHsc /O2 /utf-8 sudoku.cpp user32.lib gdi32.lib comctl32.lib /Fe:sudoku.exe
```
