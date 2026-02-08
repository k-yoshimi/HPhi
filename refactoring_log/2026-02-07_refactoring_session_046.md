Date: 2026-02-07
Time: 00:45:47 JST

# Refactoring Session 046 (SpinGC characterization extension)

## 実施内容
- `test/unit/diagonalcalc_spingc_characterization.sh` を追加。
  - 条件: `L=3`, `model="SpinGC"`, `method="FullDiag"`, `lattice="chain"`, `J=1.0`
  - 検証:
    - `output/CHECK_Memory.dat` の `idim_max=8`
    - `output/zvo_phys.dat` の行数（header含め9行）
    - 先頭3固有値（`-0.750000`）の一致
- `test/unit/CMakeLists.txt` に `unit_diagonalcalc_spingc_characterization` を追加。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (18/18)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (78/78)

## 補足
- tJ model は standard mode では未対応（`model="tJ"` で unsupported）だったため、
  characterization 追加には expert mode 入力の fixture 化が必要。
