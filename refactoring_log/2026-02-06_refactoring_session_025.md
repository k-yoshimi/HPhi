Date: 2026-02-06
Time: 22:51:46 JST

# Refactoring Session 025 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalHund` を段階分割。
- 追加 helper:
  - `ApplyHundOneInterProcess`
  - `ApplyHundIntraProcess`
- `SetDiagonalHund` は、既存の3条件を dispatcher で呼び分ける構造へ整理。
  - 両サイト inter-process: `ApplyHundBothInterProcess`
  - 片側 inter-process: `ApplyHundOneInterProcess`
  - 両サイト intra-process: `ApplyHundIntraProcess`

## 挙動維持
- modelごとのビット判定と加算式を変更せずに移設。
- `Spin/SpinGC` の `ibit` 条件 (`==0` または `==is_up`) も維持。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)
