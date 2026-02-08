Date: 2026-02-07
Time: 00:20:47 JST

# Refactoring Session 041 (sz/diagonalcalc continuation)

## 実施内容
- `src/diagonalcalc.c` の `ApplyTETransferIntraProcess` を model別 helper へ分割。
  - `GetTETransferMask`
  - `AccumulateTETransferHubbardGC`
  - `AccumulateTETransferRestrictedBasis`
  - `AccumulateTETransferSpinGC`
  - `AccumulateTETransferSpin`
- `ApplyTETransferIntraProcess` は dispatcher + 集計 (`SumMPI_dc`) の責務に限定。
- 新規 unit characterization:
  - `test/unit/sz_spingc_characterization.sh`
  - `SpinGC` の `CHECK_Sdim.dat` / `CHECK_Memory.dat` を検証。
- `test/unit/CMakeLists.txt` へ `unit_sz_spingc_characterization` を登録。

## 挙動維持
- 既存の `SetDiagonalTETransfer` 経路は数値更新式を維持。
- `SpinGC` の `sz` 次元算出結果を新規 test で固定化。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (16/16)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (76/76)
