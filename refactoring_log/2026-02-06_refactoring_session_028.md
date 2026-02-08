Date: 2026-02-06
Time: 23:01:16 JST

# Refactoring Session 028 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalTEInterAll` を段階分割。
- 追加 helper:
  - `ApplyTEInterAllBothInterProcess`
  - `ApplyTEInterAllOneInterProcess`
  - `ApplyTEInterAllIntraProcess`
- `SetDiagonalTEInterAll` は以下3分岐の dispatcher へ整理。
  - 両サイト inter-process: `ApplyTEInterAllBothInterProcess`
  - 片側 inter-process: `ApplyTEInterAllOneInterProcess`
  - 両サイト intra-process: `ApplyTEInterAllIntraProcess`

## 挙動維持
- model別処理と条件分岐（Hubbard/Kondo/tJ, Spin/SpinGC, general spin）を変更せず移設。
- `isite1/isite2` と `isigma1/isigma2` の swap 条件を維持。
- `dam_pr = SumMPI_dc(dam_pr)` と `X->Large.prdct += dam_pr` の更新を各経路で維持。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)
