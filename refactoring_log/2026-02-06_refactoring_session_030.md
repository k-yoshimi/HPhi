Date: 2026-02-06
Time: 23:08:46 JST

# Refactoring Session 030 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalTETransfer` を段階分割。
- 追加 helper:
  - `ApplyTETransferInterProcess`
  - `ApplyTETransferIntraProcess`
- `SetDiagonalTETransfer` は以下2分岐の dispatcher へ整理。
  - inter-process: `ApplyTETransferInterProcess`
  - intra-process: `ApplyTETransferIntraProcess`

## 挙動維持
- model別処理（Hubbard/Kondo/tJ, Spin/SpinGC, general spin）を変更せず移設。
- `dam_pr = SumMPI_dc(dam_pr)` と `X->Large.prdct += dam_pr` 更新を維持。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)
