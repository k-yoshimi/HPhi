Date: 2026-02-06
Time: 22:48:34 JST

# Refactoring Session 024 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalHund` を段階分割。
- 追加 helper:
  - `ApplyHundBothInterProcess`
- 対象:
  - `isite1 > X->Def.Nsite`（両サイト inter-process）分岐のみを helper 化。

## 挙動維持
- Hubbard/Kondo/tJ 系の `num1_up/down`, `num2_up/down` による寄与式を維持。
- Spin 系の `ibit == 0 || ibit == is_up` 条件を維持。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)
