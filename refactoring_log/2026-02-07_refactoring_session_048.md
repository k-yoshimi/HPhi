Date: 2026-02-07
Time: 00:55:41 JST

# Refactoring Session 048 (tJGC expert fixture + check fix)

## 実施内容
- `src/check.c`
  - `tJGC` の `idim_max` 計算を `Ne` 依存から `Nsite` 依存へ修正。
  - これにより expert mode で `Ne=0` の既定状態でも、`tJGC` の Hilbert 空間次元（`3^Nsite`）を正しく算出。
- 新規: `test/unit/sz_tjgc_expert_characterization.sh`
  - expert mode `.def` fixture（`CalcModel=10`）をテスト内生成。
  - `Nsite=4` で `sdim=16` / `idim_max=81` / `Err_sz.dat` 非生成を検証。
- `test/unit/CMakeLists.txt`
  - `unit_sz_tjgc_expert_characterization` を追加（label: `unit`）。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (20/20)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (80/80)
