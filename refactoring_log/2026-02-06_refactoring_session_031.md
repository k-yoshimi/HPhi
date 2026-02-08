Date: 2026-02-06
Time: 23:11:26 JST

# Refactoring Session 031 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalChemi` を段階分割。
- 追加 helper:
  - `ApplyChemiIntraProcess`
- `SetDiagonalChemi` は以下2分岐の dispatcher へ整理。
  - inter-process: `ApplyChemiInterProcess`
  - intra-process: `ApplyChemiIntraProcess`

## 挙動維持
- model別処理（Hubbard/Kondo/tJ, Spin/SpinGC, general spin）を変更せず移設。
- `list_Diagonal` 更新条件と更新式を維持。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)
