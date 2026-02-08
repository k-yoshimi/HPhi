Date: 2026-02-06
Time: 22:38:47 JST

# Refactoring Session 021 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalChemi` を段階分割。
- 新規 helper:
  - `ApplyChemiInterProcess`
- 変更点:
  - `SetDiagonalChemi` の inter-process 分岐 (`isite1 > X->Def.Nsite`) を helper 化。
  - intra-process 分岐 (`isite1 <= X->Def.Nsite`) は既存実装を維持。

## 目的
- 巨大関数の責務境界を明確化し、次段の分割（`SetDiagonalCoulombInter` など）を進めやすくする。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)
