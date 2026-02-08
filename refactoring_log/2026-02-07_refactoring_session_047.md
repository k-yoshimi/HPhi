Date: 2026-02-07
Time: 00:49:53 JST

# Refactoring Session 047 (tJ expert-mode fixture test)

## 実施内容
- `test/unit/sz_tj_expert_characterization.sh` を新規追加。
  - standard mode 非対応の `tJ` を、expert mode `.def` fixture で検証する構成。
  - 同一テストで 2 ケースを実行:
    - `2Sz` あり: tJ
    - `2Sz` なし: tJNConserved
- `test/unit/CMakeLists.txt` に `unit_sz_tj_expert_characterization` を追加。

## 検証項目
- tJ (`2Sz` あり)
  - `output/CHECK_Sdim.dat`: `sdim=16 =2^4`
  - `output/CHECK_Memory.dat`: `idim_max=12`
- tJNConserved (`2Sz` なし)
  - `output/CHECK_Sdim.dat`: `sdim=16 =2^4`
  - `output/CHECK_Memory.dat`: `idim_max=32`

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (19/19)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (79/79)
