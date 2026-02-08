Date: 2026-02-06
Time: 22:30:33 JST

# Refactoring Session 019 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `diagonalcalcForTE` を helper 分割。
- 追加 helper:
  - `ApplyTETransferDiagonalStep`
  - `ApplyTEInterAllDiagonalStep`
  - `ApplyTEChemiStep`
- 本体関数では、helper の組み合わせで同じ分岐挙動を表現。

## 挙動維持のポイント
- 既存コードの `if (...) {transfer} else if (...) {interall + chemi}` を維持。
- つまり `NTETransferDiagonal[_istep] > 0` のときは、`NTEInterAllDiagonal`/`NTEChemi` は処理しない。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)
